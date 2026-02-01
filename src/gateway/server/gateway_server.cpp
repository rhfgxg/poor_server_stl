#include "gateway_server.h"

GatewayServerImpl::GatewayServerImpl(LoggerManager& logger_manager_, const std::string address, const std::string port)
    : BaseServer(rpc_server::ServerType::GATEWAY, address, port, logger_manager_, 4),
      login_connection_pool(10),
      file_connection_pool(10),
      gateway_connection_pool(10),
      db_connection_pool(10)
{
}

GatewayServerImpl::~GatewayServerImpl()
{
    pool_update_running_.store(false);
    if (update_pool_thread_.joinable())
    {
        update_pool_thread_.join();
    }
    
    // 停止 Skynet TCP 管理器
    if (skynet_tcp_manager_)
    {
        skynet_tcp_manager_->stop_server();
        skynet_tcp_manager_->disconnect();
    }
    
    log_shutdown("GatewayServer stopped");
}

// ==================== BaseServer 钩子方法 ====================

bool GatewayServerImpl::on_start()
{
    log_startup("GatewayServer initializing...");

    if (!redis_client.connect_from_config())
    {
        log_startup("Failed to connect to Redis");
        return false;
    }

    log_startup("GatewayServer initialized successfully");
    return true;
}

void GatewayServerImpl::on_stop()
{
    log_shutdown("GatewayServer shutting down...");
    
    // 停止 Skynet TCP 管理器
    if (skynet_tcp_manager_)
    {
        skynet_tcp_manager_->stop_server();
        skynet_tcp_manager_->disconnect();
    }
    
    pool_update_running_.store(false);
    if (update_pool_thread_.joinable())
    {
        update_pool_thread_.join();
    }
    
    log_shutdown("GatewayServer shutdown complete");
}

void GatewayServerImpl::on_registered(bool success)
{
    if (success)
    {
        Init_connection_pool();
        
        // 初始化 Skynet TCP 管理器（统一服务器和客户端功能）
        skynet_tcp_manager_ = std::make_unique<SkynetTcpManager>(logger_manager_, central_connection_pool_.get());
        
        // 设置服务器模式回调（接收 Skynet 请求）
        skynet_tcp_manager_->set_request_handler([this](uint16_t msg_type, const std::string& payload) {
            return Handle_skynet_request(msg_type, payload);
        });
        
        // 设置推送消息回调（接收 Skynet 主动推送）
        skynet_tcp_manager_->set_push_callback([this](const SkynetTcpMessage& msg) {
            Handle_skynet_push(msg);
        });
        
        // 更新白名单并启动服务器模式
        skynet_tcp_manager_->update_whitelist();
        skynet_tcp_manager_->start_server(8889);
        
        pool_update_running_.store(true);
        update_pool_thread_ = std::thread(&GatewayServerImpl::Update_connection_pool, this);
        
        log_startup("Gateway connection pools initialized and update thread started");
        log_startup("Skynet TCP manager started on port 8889");
    }
    else
    {
        log_startup("Failed to register, skipping connection pool initialization");
    }
}

// ==================== 连接池管理 ====================

void GatewayServerImpl::Init_connection_pool()
{
    try
    {
        auto channel = central_connection_pool_->get_connection(rpc_server::ServerType::CENTRAL);
        auto stub = rpc_server::CentralServer::NewStub(channel);
        
        rpc_server::MultipleConnectPoorReq req;
        req.add_server_types(rpc_server::ServerType::LOGIN);
        req.add_server_types(rpc_server::ServerType::FILE);
        req.add_server_types(rpc_server::ServerType::GATEWAY);
        req.add_server_types(rpc_server::ServerType::DB);
        
        rpc_server::MultipleConnectPoorRes res;
        grpc::ClientContext context;
        
        grpc::Status status = stub->Get_connec_poor(&context, req, &res);
        
        central_connection_pool_->release_connection(rpc_server::ServerType::CENTRAL, channel);
        
        if (status.ok() && res.success())
        {
            for (const rpc_server::ConnectPool& connect_pool : res.connect_pools())
            {
                for (const rpc_server::ConnectInfo& conn_info : connect_pool.connect_info())
                {
                    switch (connect_pool.server_type())
                    {
                    case rpc_server::ServerType::LOGIN:
                        login_connection_pool.add_server(
                            rpc_server::ServerType::LOGIN,
                            conn_info.address(),
                            std::to_string(conn_info.port())
                        );
                        get_logger(poor::LogCategory::CONNECTION_POOL)->info("Added LOGIN server: {}:{}", 
                            conn_info.address(), conn_info.port());
                        break;
                        
                    case rpc_server::ServerType::FILE:
                        file_connection_pool.add_server(
                            rpc_server::ServerType::FILE,
                            conn_info.address(),
                            std::to_string(conn_info.port())
                        );
                        get_logger(poor::LogCategory::CONNECTION_POOL)->info("Added FILE server: {}:{}", 
                            conn_info.address(), conn_info.port());
                        break;
                        
                    case rpc_server::ServerType::GATEWAY:
                        gateway_connection_pool.add_server(
                            rpc_server::ServerType::GATEWAY,
                            conn_info.address(),
                            std::to_string(conn_info.port())
                        );
                        get_logger(poor::LogCategory::CONNECTION_POOL)->info("Added GATEWAY server: {}:{}", 
                            conn_info.address(), conn_info.port());
                        break;
                    
                    case rpc_server::ServerType::DB:
                        db_connection_pool.add_server(
                            rpc_server::ServerType::DB,
                            conn_info.address(),
                            std::to_string(conn_info.port())
                        );
                        get_logger(poor::LogCategory::CONNECTION_POOL)->info("Added DB server: {}:{}", 
                            conn_info.address(), conn_info.port());
                        break;
                        
                    default:
                        break;
                    }
                }
            }
            
            get_logger(poor::LogCategory::CONNECTION_POOL)->info("Gateway connection pools initialized successfully");
        }
        else
        {
            get_logger(poor::LogCategory::CONNECTION_POOL)->error("Failed to get connection pools information");
        }
    }
    catch (const std::exception& e)
    {
        get_logger(poor::LogCategory::CONNECTION_POOL)->error("Exception during connection pool initialization: {}", e.what());
    }
}

void GatewayServerImpl::Update_connection_pool()
{
    while (pool_update_running_.load())
    {
        std::this_thread::sleep_for(std::chrono::minutes(5));
        
        if (pool_update_running_.load())
        {
            Init_connection_pool();
            
            // 同时更新 Skynet IP 白名单
            if (skynet_tcp_manager_)
            {
                skynet_tcp_manager_->update_whitelist();
        }
    }
}
}

// ==================== gRPC 服务接口 ====================
// 客户端注册
grpc::Status GatewayServerImpl::Client_register(grpc::ServerContext* context [[maybe_unused]], const rpc_server::ClientRegisterReq* req, rpc_server::ClientRegisterRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        std::string client_address = req->address();
        
        auto connections = gateway_connection_pool.get_all_connections();
        
        for (const auto& connection : connections[rpc_server::ServerType::GATEWAY])
        {
            rpc_server::GatewayConnectInfo* conn_info = res->add_connect_info();
            conn_info->set_address(connection.first);
            conn_info->set_port(std::stoi(connection.second));
        }
        
        std::string client_token = "123456";
        
        res->set_client_token(client_token);
        res->set_success(true);
        res->set_message("Connection pools information retrieved successfully");
        
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Client successfully registered: {}", client_address);
        
            // Redis 操作添加异常处理
            try
            {
        auto client = redis_client.get_client();
        client->set(client_address + "_status", "online");
        client->expire(client_address + "_status", 1800);
        client->set(client_address + "_token", client_token);
        client->expire(client_address + "_token", 1800);
            }
            catch (const std::exception& redis_err)
            {
                get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error("Redis operation failed: {}", redis_err.what());
                // Redis 失败不影响注册结果
            }
    });
    
    task_future.get();
    return grpc::Status::OK;
}

// 请求转发
grpc::Status GatewayServerImpl::Request_forward(grpc::ServerContext* context [[maybe_unused]], const rpc_server::ForwardReq* req, rpc_server::ForwardRes* res)
{
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Request_forward received, service_type={}", static_cast<int>(req->service_type()));
    
    auto task_future = submit_task([this, req, res]() {
        switch (req->service_type())
        {
        case rpc_server::ServiceType::REQ_REGISTER:
            Forward_to_register_service(req->payload(), res);
            break;
            
        case rpc_server::ServiceType::REQ_LOGIN:
            get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Forwarding login request...");
            Forward_to_login_service(req->payload(), res);
            break;
            
        case rpc_server::ServiceType::REQ_LOGOUT:
            Forward_to_logout_service(req->payload(), res);
            break;
            
        case rpc_server::ServiceType::REQ_CHANGE_PASSWORD:
            Forward_to_change_password_service(req->payload(), res);
            break;
            
        case rpc_server::ServiceType::REQ_FILE_TRANSMISSION_READY:
            Forward_to_file_transmission_ready_service(req->payload(), res);
            break;
            
        case rpc_server::ServiceType::REQ_FILE_DELETE:
            Forward_to_file_delete_service(req->payload(), res);
            break;
            
        case rpc_server::ServiceType::REQ_FILE_LIST:
            Forward_to_file_list_service(req->payload(), res);
            break;
        
        // Skynet 游戏服务转发
        case rpc_server::ServiceType::REQ_ENTER_GAME:
            Forward_to_enter_game_service(req->payload(), res);
            break;
            
        case rpc_server::ServiceType::REQ_LEAVE_GAME:
            Forward_to_leave_game_service(req->payload(), res);
                break;
            
            // Skynet 玩家服务转发
            case rpc_server::ServiceType::REQ_PLAYER_ACTION:
                Forward_to_player_action_service(req->payload(), res);
                break;
            
            case rpc_server::ServiceType::REQ_GET_PLAYER_STATUS:
                Forward_to_player_status_service(req->payload(), res);
                break;
            
            // Skynet 测试服务
            case rpc_server::ServiceType::REQ_ECHO:
                Forward_to_echo_service(req->payload(), res);
                break;
        
        // 注意：DB 转发 (REQ_DB_CREATE/READ/UPDATE/DELETE) 不开放给外部客户端
        // 只能由内部服务（Skynet）通过内部 TCP 端口调用
        // 参见 gateway_forward_db.cpp 中的 Internal_forward_* 函数
            
        default:
            res->set_success(false);
            // ForwardRes 没有 message 字段，只需设置 success 为 false
            break;
        }
    });
    
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status GatewayServerImpl::Client_heartbeat(grpc::ServerContext* context [[maybe_unused]], const rpc_server::ClientHeartbeatReq* req [[maybe_unused]], rpc_server::ClientHeartbeatRes* res [[maybe_unused]])
{
    return grpc::Status::OK;
}

// 获取网关服务器连接池
grpc::Status GatewayServerImpl::Get_gateway_pool(grpc::ServerContext* context [[maybe_unused]], const rpc_server::GetGatewayPoolReq* req, rpc_server::GetGatewayPoolRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_return_gateway_poor(req, res);
    });
    
    task_future.get();
    return grpc::Status::OK;
}

// ==================== 请求转发处理函数 ====================
// 获取网关服务器连接池信息
grpc::Status GatewayServerImpl::Handle_return_gateway_poor(const rpc_server::GetGatewayPoolReq* req [[maybe_unused]], rpc_server::GetGatewayPoolRes* res)
{
    auto connections = gateway_connection_pool.get_all_connections();
    
    for (const auto& connection : connections[rpc_server::ServerType::GATEWAY])
    {
        rpc_server::GatewayConnectInfo* conn_info = res->add_connect_info();
        conn_info->set_address(connection.first);
        conn_info->set_port(std::stoi(connection.second));
    }
    
    res->set_success(true);
    res->set_message("Connection pools information retrieved successfully");
    
    return grpc::Status::OK;
}

// 注意：转发函数的实现已经移到以下文件：
// - gateway_forward_login.cpp (登录服务转发)
// - gateway_forward_file.cpp (文件服务转发)
