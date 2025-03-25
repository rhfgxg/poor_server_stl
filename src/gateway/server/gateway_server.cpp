#include "gateway_server.h"

GatewayServerImpl::GatewayServerImpl(LoggerManager& logger_manager_, const std::string address, const std::string port):
    server_type(rpc_server::ServerType::GATEWAY),
    server_address(address),
    server_port(port),
    logger_manager(logger_manager_),  
    central_stub(rpc_server::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))), // 中心服务器存根
    login_connection_pool(10),
    file_connection_pool(10),
    gateway_connection_pool(10)
{
    this->register_server();

    // 定时向中心服务器获取最新的连接池状态
    std::thread(&GatewayServerImpl::Update_connection_pool, this).detach();
    // 定时向中心服务器发送心跳包
    std::thread(&GatewayServerImpl::Send_heartbeat, this).detach();
}

GatewayServerImpl::~GatewayServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    unregister_server(); // 注销服务器

    // 记录关闭日志
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("GatewayServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

/*************************************** 多线程工具函数 *****************************************************************/
// 启动线程池
void GatewayServerImpl::start_thread_pool(int num_threads)
{// 相关注释请参考 /src/central/src/central/central_server.cpp/start_thread_pool()
    for(int i = 0; i < num_threads; ++i)
    {
        this->thread_pool.emplace_back(&GatewayServerImpl::Worker_thread, this);   // 创建线程
    }
}

// 停止线程池
void GatewayServerImpl::stop_thread_pool()
{// 相关注释请参考 /src/central/src/central/central_server.cpp/stop_thread_pool()
    {
        std::lock_guard<std::mutex> lock(this->queue_mutex);
        this->stop_threads = true;
    }

    this->queue_cv.notify_all();

    for(auto& thread : this->thread_pool)
    {
        if(thread.joinable())
        {
            thread.join();
        }
    }
    this->thread_pool.clear();

    std::queue<std::function<void()>> empty;
    std::swap(this->task_queue,empty);
}

// 添加异步任务
std::future<void> GatewayServerImpl::add_async_task(std::function<void()> task)
{
    auto task_ptr = std::make_shared<std::packaged_task<void()>>(std::move(task));
    std::future<void> task_future = task_ptr->get_future();

    {
        std::lock_guard<std::mutex> lock(this->queue_mutex);
        this->task_queue.push([task_ptr]() {
            (*task_ptr)();
        });
    }
    this->queue_cv.notify_one();
    return task_future;
}

// 线程池工作函数
void GatewayServerImpl::Worker_thread()
{// 相关注释请参考 /src/central/src/central/central_server.cpp/worker_thread()
    while(true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);

            this->queue_cv.wait(lock,[this] {
                return !this->task_queue.empty() || this->stop_threads;
            });

            if(this->stop_threads && this->task_queue.empty())
            {
                return;
            }
            task = std::move(this->task_queue.front());
            this->task_queue.pop();
        }
        task();
    }
}

/*************************************** 连接池管理 *********************************************************/
// 服务器注册
void GatewayServerImpl::register_server()
{
    // 请求
    rpc_server::RegisterServerReq request;
    request.set_server_type(rpc_server::ServerType::GATEWAY);
    request.set_address(this->server_address);
    request.set_port(this->server_port);
    // 响应
    rpc_server::RegisterServerRes response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = this->central_stub->Register_server(&context, request, &response);

    if(status.ok() && response.success())
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Gateway server registered successfully: {} {}", this->server_address, this->server_port);
        this->Init_connection_pool(); // 初始化连接池
    }
    else
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->error("Gateway server registration failed: {} {}", this->server_address, this->server_port);
    }
}

// 注销服务器
void GatewayServerImpl::unregister_server()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    rpc_server::UnregisterServerReq request;
    request.set_server_type(rpc_server::ServerType::GATEWAY);
    request.set_address(this->server_address);
    request.set_port(this->server_port);

    // 响应
    rpc_server::UnregisterServerRes response;

    grpc::Status status = this->central_stub->Unregister_server(&context, request, &response);

    if(status.ok() && response.success())
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Gateway server unregistered successfully: {} {}", this->server_address, this->server_port);
    }
    else
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->error("ERROR：Gateway server unregistration failed: {} {}", this->server_address, this->server_port);
    }
}

// 初始化/更新连接池
void GatewayServerImpl::Init_connection_pool()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    rpc_server::MultipleConnectPoorReq req;
    req.add_server_types(rpc_server::ServerType::LOGIN);
    req.add_server_types(rpc_server::ServerType::FILE);
    req.add_server_types(rpc_server::ServerType::GATEWAY);
    // 响应
    rpc_server::MultipleConnectPoorRes res;

    grpc::Status status = central_stub->Get_connec_poor(&context, req, &res);

    if(status.ok() && res.success())
    {
        for(const rpc_server::ConnectPool& connect_pool : *res.mutable_connect_pools())
        {
            for(const rpc_server::ConnectInfo& conn_info : connect_pool.connect_info())
            {
                switch(connect_pool.server_type())
                {
                case rpc_server::ServerType::LOGIN:
                {
                    login_connection_pool.add_server(rpc_server::ServerType::LOGIN, conn_info.address(), std::to_string(conn_info.port()));
                    break;
                }
                case rpc_server::ServerType::FILE:
                {
                    file_connection_pool.add_server(rpc_server::ServerType::FILE, conn_info.address(), std::to_string(conn_info.port()));
                    break;
                }
                case rpc_server::ServerType::GATEWAY:
                {
                    gateway_connection_pool.add_server(rpc_server::ServerType::GATEWAY, conn_info.address(), std::to_string(conn_info.port()));
                    break;
                }
                default:
                break;
                }
            }
        }
        logger_manager.getLogger(poor::LogCategory::CONNECTION_POOL)->info("Gateway server updated connection pools successfully");
    }
    else
    {
        logger_manager.getLogger(poor::LogCategory::CONNECTION_POOL)->error("Failed to get connection pools information");
    }
}

/******************************************* 定时任务 *****************************************************/
// 定时任务：更新连接池
void GatewayServerImpl::Update_connection_pool()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::minutes(5)); // 每5分钟更新一次连接池
        Init_connection_pool();
    }
}

// 定时任务：发送心跳包
void GatewayServerImpl::Send_heartbeat()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒发送一次心跳包

        rpc_server::HeartbeatReq request;
        rpc_server::HeartbeatRes response;
        grpc::ClientContext context;

        request.set_server_type(rpc_server::ServerType::GATEWAY);
        request.set_address("127.0.0.1"); // 设置服务器ip
        request.set_port("50051"); // 设置服务器端口

        grpc::Status status = central_stub->Heartbeat(&context,request,&response);

        if(status.ok() && response.success())
        {
            logger_manager.getLogger(poor::LogCategory::HEARTBEAT)->info("Heartbeat sent successfully.");
        }
        else
        {
            logger_manager.getLogger(poor::LogCategory::HEARTBEAT)->error("Failed to send heartbeat.");
        }
    }
}

/**************************************** grpc服务接口定义 **************************************************************************/
// 客户端注册
grpc::Status GatewayServerImpl::Client_register(grpc::ServerContext* context, const rpc_server::ClientRegisterReq* req, rpc_server::ClientRegisterRes* res)
{
    auto task_future = this->add_async_task([this, context, req, res] {  // 加入任务队列
        // 获取客户端信息
        std::string client_address = req->address();

        // 获取网关连接池中的所有连接
        std::unordered_map<rpc_server::ServerType, std::vector<std::pair<std::string, std::string>>> connections = gateway_connection_pool.get_all_connections();

        // 遍历连接池中的所有连接
        for(const auto& connection : connections[rpc_server::ServerType::GATEWAY])
        {
            rpc_server::GatewayConnectInfo* conn_info = res->add_connect_info();
            conn_info->set_address(connection.first);
            conn_info->set_port(std::stoi(connection.second));
        }

        std::string client_token = "123456"; // 生成客户端令牌

        res->set_client_token(client_token); // 设置客户端令牌
        res->set_success(true);
        res->set_message("Connection pools information retrieved successfully");

        this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Client: successfully registered a server: {}", client_address);

        // 将用户在线状态存储到Redis中
        auto client = redis_client.get_client();
        client->set(client_address + "_status", "online");
        client->expire(client_address + "_status", 1800); // 设置30分钟过期时间
        client->set(client_address + "_token", client_token);
        client->expire(client_address + "_token", 1800);
    });

    task_future.get();
    return grpc::Status::OK;
}

// 服务转发接口
grpc::Status GatewayServerImpl::Request_forward(grpc::ServerContext* context, const rpc_server::ForwardReq* req, rpc_server::ForwardRes* res)
{
    auto task_future = this->add_async_task([this,req,res] {
        switch(req->service_type()) // 根据请求的服务类型进行转发
        {
        case rpc_server::ServiceType::REQ_LOGIN: // 用户登录请求
        {
            Forward_to_login_service(req->payload(), res);  // 解析负载，并转发到对应服务
            break;
        }
        case rpc_server::ServiceType::REQ_LOGOUT:   // 用户登出请求
        {
            Forward_to_logout_service(req->payload(), res);
            break;
        }
        case rpc_server::ServiceType::REQ_REGISTER:    // 用户注册请求
        {
            Forward_to_register_service(req->payload(), res);
            break;
        }
        case rpc_server::ServiceType::REQ_FILE_UPLOAD_READY:    // 文件上传准备
        {
            // Forward_to_file_upload_ready_service(req->payload(), res);
            break;
        }
        case rpc_server::ServiceType::REQ_FILE_UPLOAD:  // 文件上传
        {
            // Forward_to_file_upload_service(req->payload(), res);
            break;
        }
        case rpc_server::ServiceType::REQ_FILE_DOWNLOAD:    // 文件下载
        {
            // Forward_to_file_download_service(req->payload(), res);
            break;
        }
        case rpc_server::ServiceType::REQ_FILE_DELETE:  // 文件删除
        {
            // Forward_to_file_delete_service(req->payload(), res);
            break;
        }
        case rpc_server::ServiceType::REQ_FILE_LIST:    // 获取文件列表
        {
            // Forward_to_file_list_service(req->payload(), res);
            break;
        }
        default:    // 未知服务类型
        {
            res->set_success(false);
            break;
        }
        }
    });

    // 等待任务完成
    task_future.get();

    return grpc::Status::OK;
}

// 接收客户端心跳
grpc::Status GatewayServerImpl::Client_heartbeat(grpc::ServerContext* context, const rpc_server::ClientHeartbeatReq* req, rpc_server::ClientHeartbeatRes* res)
{
    return grpc::Status::OK;
}

// 发送网关服务器连接池
grpc::Status GatewayServerImpl::Get_Gateway_pool(grpc::ServerContext* context, const rpc_server::GetGatewayPoolReq* req, rpc_server::GetGatewayPoolRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {  // 加入任务队列
        this->Handle_return_gateway_poor(req, res);
    });

    task_future.get();
    return grpc::Status::OK;
}

/**************************************** grpc服务接口工具函数 **************************************************************************/
// Login 方法，处理登录请求
grpc::Status GatewayServerImpl::Forward_to_login_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    rpc_server::LoginReq login_req;  // 创建登录请求对象
    bool req_out = login_req.ParseFromString(payload); // 将负载解析为登录请求对象

    if(!req_out) // 如果解析失败
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Failed to parse LoginRequest");
    }

    // 构造响应
    rpc_server::LoginRes login_response;
    grpc::ClientContext context;

    // 获取连接池中的连接
    auto channel = this->login_connection_pool.get_connection(rpc_server::ServerType::LOGIN);
    auto login_stub = rpc_server::LoginServer::NewStub(channel);

    grpc::Status status = login_stub->Login(&context, login_req, &login_response);

    if(!status.ok()) // 如果调用失败
    {
        return status;
    }

    bool response_out = login_response.SerializeToString(res->mutable_response()); // 将登录响应序列化为转发响应
    if(!response_out) // 如果序列化失败
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to serialize LoginResponse");
    }

    if(login_response.success()) // 如果登录成功
    {
        res->set_success(true);    // 设置响应对象 response 的 success 字段为 true
    }
    else
    {
        res->set_success(false);
    }

    this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Forward REQ successfully: user login");

    this->login_connection_pool.release_connection(rpc_server::ServerType::LOGIN, channel); // 释放连接
    return grpc::Status::OK;
}

// Register 方法，处理注册请求
grpc::Status GatewayServerImpl::Forward_to_register_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    rpc_server::RegisterReq register_req;  // 创建注册请求对象
    rpc_server::RegisterRes register_res;
    grpc::ClientContext context;

    bool req_out = register_req.ParseFromString(payload); // 将负载解析为登录请求对象

    if(!req_out) // 如果解析失败
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Failed to parse RegisterRequest");
    }

    // 获取连接池中的连接
    auto channel = this->login_connection_pool.get_connection(rpc_server::ServerType::LOGIN);
    auto login_stub = rpc_server::LoginServer::NewStub(channel);

    grpc::Status status = login_stub->Register(&context, register_req, &register_res);

    if(!status.ok()) // 如果调用失败
    {
        return status;
    }

    bool res_out = register_res.SerializeToString(res->mutable_response()); // 将登录响应序列化为转发响应
    if(!res_out) // 如果序列化失败
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to serialize RegisterResponse");
    }

    if(register_res.success()) // 如果登录成功
    {
        res->set_success(true);    // 设置响应对象 response 的 success 字段为 true
    }
    else
    {
        res->set_success(false);
    }

    this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Forward REQ successfully: user register");

    this->login_connection_pool.release_connection(rpc_server::ServerType::LOGIN, channel); // 释放连接
    return grpc::Status::OK;
}

// Logout 方法，处理登出请求
grpc::Status GatewayServerImpl::Forward_to_logout_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    rpc_server::LogoutReq logout_req;  // 创建注册请求对象
    rpc_server::LogoutRes logout_res;
    grpc::ClientContext context;

    bool req_out = logout_req.ParseFromString(payload); // 将负载解析为登录请求对象

    if(!req_out) // 如果解析失败
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Failed to parse LogoutRequest");
    }

    // 获取连接池中的连接
    auto channel = this->login_connection_pool.get_connection(rpc_server::ServerType::LOGIN);
    auto login_stub = rpc_server::LoginServer::NewStub(channel);

    grpc::Status status = login_stub->Logout(&context, logout_req, &logout_res);

    if(!status.ok()) // 如果调用失败
    {
        return status;
    }

    bool res_out = logout_res.SerializeToString(res->mutable_response()); // 将登录响应序列化为转发响应
    if(!res_out) // 如果序列化失败
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to serialize LogoutResponse");
    }

    if(logout_res.success()) // 如果登录成功
    {
        res->set_success(true);    // 设置响应对象 response 的 success 字段为 true
    } else
    {
        res->set_success(false);
    }

    this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Forward REQ successfully: user logout");

    this->login_connection_pool.release_connection(rpc_server::ServerType::LOGIN, channel); // 释放连接
    return grpc::Status::OK;
}

// 发送网关服务器连接池
grpc::Status GatewayServerImpl::Handle_return_gateway_poor(const rpc_server::GetGatewayPoolReq* req, rpc_server::GetGatewayPoolRes* res)
{
    // 获取网关连接池中的所有连接
    std::unordered_map<rpc_server::ServerType, std::vector<std::pair<std::string, std::string>>> connections = gateway_connection_pool.get_all_connections();

    // 遍历连接池中的所有连接
    for(const auto& connection : connections[rpc_server::ServerType::GATEWAY])
    {
        rpc_server::GatewayConnectInfo* conn_info = res->add_connect_info();
        conn_info->set_address(connection.first);
        conn_info->set_port(std::stoi(connection.second));
    }

    res->set_success(true);
    res->set_message("Connection pools information retrieved successfully");

    return grpc::Status::OK;
}
/******************************************** 其他工具函数 ***********************************************/
