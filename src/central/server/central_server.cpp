#include "central_server.h"

// 构造函数 - 继承 BaseServer
CentralServerImpl::CentralServerImpl(LoggerManager& logger_manager_, const std::string& address, const std::string& port):
    BaseServer(rpc_server::ServerType::CENTRAL, address, port, logger_manager_, 4),  // 调用基类构造函数，4个线程
    heartbeat_checker_running_(false),
    battle_connection_pool_(10),
    central_connection_pool_(10),
    db_connection_pool_(10),
    file_connection_pool_(10),
    gateway_connection_pool_(10),
    logic_connection_pool_(10),
    login_connection_pool_(10),
    matching_connection_pool_(10)
{
    // 连接池初始化日志
    log_activity("Battle_connection_pool: initialized, size: 10");
    log_activity("Central_connection_pool: initialized, size: 10");
    log_activity("DB_connection_pool: initialized, size: 10");
    log_activity("Gateway_connection_pool: initialized, size: 10");
    log_activity("Logic_connection_pool: initialized, size: 10");
    log_activity("Login_connection_pool: initialized, size: 10");
    log_activity("File_connection_pool: initialized, size: 10");
    log_activity("Matching_connection_pool: initialized, size: 10");

    // 将本服务器加入中心连接池
    central_connection_pool_.add_server(
        rpc_server::ServerType::CENTRAL,
        get_server_address(),
        get_server_port()
    );
    log_activity("Central server added to connection pool: " + get_server_address() + ":" + get_server_port());
}

// 析构函数
CentralServerImpl::~CentralServerImpl()
{
    stop_heartbeat_checker();
    log_shutdown("Central_server stopped");
}

// ==================== BaseServer 钩子方法 ====================

bool CentralServerImpl::on_start()
{
    log_startup("CentralServer initializing...");
    
    // 启动心跳检查线程
    start_heartbeat_checker();
    
    log_startup("CentralServer initialized successfully");
    return true;
}

void CentralServerImpl::on_stop()
{
    log_shutdown("CentralServer shutting down...");
    
    // 停止心跳检查线程
    stop_heartbeat_checker();
    
    log_shutdown("CentralServer shutdown complete");
}

// ==================== gRPC 服务接口 ====================

grpc::Status CentralServerImpl::Register_server(
    grpc::ServerContext* context,
    const rpc_server::RegisterServerReq* req,
    rpc_server::RegisterServerRes* res)
{
    // 使用 BaseServer 的 submit_task 提交异步任务
    auto task_future = submit_task([this, req, res]() {
        Handle_register_server(req, res);
    });

    task_future.get();
    return grpc::Status::OK;
}

grpc::Status CentralServerImpl::Unregister_server(
    grpc::ServerContext* context,
    const rpc_server::UnregisterServerReq* req,
    rpc_server::UnregisterServerRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        rpc_server::ServerType server_type = req->server_type();
        std::string address = req->address();
        std::string port = req->port();

        Release_server_connection(server_type, address, port);

        res->set_success(true);
        res->set_message("Successfully unregistered");
    });

    task_future.get();
    return grpc::Status::OK;
}

grpc::Status CentralServerImpl::Get_connec_poor(
    grpc::ServerContext* context,
    const rpc_server::MultipleConnectPoorReq* req,
    rpc_server::MultipleConnectPoorRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        All_connec_poor(req, res);
    });

    task_future.get();
    return grpc::Status::OK;
}

grpc::Status CentralServerImpl::Heartbeat(
    grpc::ServerContext* context,
    const rpc_server::HeartbeatReq* req,
    rpc_server::HeartbeatRes* res)
{
    std::lock_guard<std::mutex> lock(heartbeat_mutex_);
    
    heartbeat_records_[req->address()] = {
        req->server_type(),
        req->address(),
        req->port(),
        std::chrono::steady_clock::now()
    };

    res->set_success(true);
    res->set_message("Heartbeat received");
    return grpc::Status::OK;
}

// ==================== gRPC 工具函数 ====================

void CentralServerImpl::Handle_register_server(
    const rpc_server::RegisterServerReq* req,
    rpc_server::RegisterServerRes* res)
{
    rpc_server::ServerType server_type = req->server_type();
    std::string address = req->address();
    std::string port = req->port();

    switch (server_type)
    {
    case rpc_server::ServerType::BATTLE:
        battle_connection_pool_.add_server(server_type, address, port);
        log_activity("Battle_connection_pool: registered " + address + ":" + port);
        break;

    case rpc_server::ServerType::CENTRAL:
        central_connection_pool_.add_server(server_type, address, port);
        log_activity("Central_connection_pool: registered " + address + ":" + port);
        break;

    case rpc_server::ServerType::DB:
        db_connection_pool_.add_server(server_type, address, port);
        log_activity("DB_connection_pool: registered " + address + ":" + port);
        break;

    case rpc_server::ServerType::GATEWAY:
        gateway_connection_pool_.add_server(server_type, address, port);
        log_activity("Gateway_connection_pool: registered " + address + ":" + port);
        break;

    case rpc_server::ServerType::LOGIC:
        logic_connection_pool_.add_server(server_type, address, port);
        log_activity("Logic_connection_pool: registered " + address + ":" + port);
        break;

    case rpc_server::ServerType::LOGIN:
        login_connection_pool_.add_server(server_type, address, port);
        log_activity("Login_connection_pool: registered " + address + ":" + port);
        break;

    case rpc_server::ServerType::FILE:
        file_connection_pool_.add_server(server_type, address, port);
        log_activity("File_connection_pool: registered " + address + ":" + port);
        break;

    case rpc_server::ServerType::MATCHING:
        matching_connection_pool_.add_server(server_type, address, port);
        log_activity("Matching_connection_pool: registered " + address + ":" + port);
        break;

    default:
        res->set_success(false);
        res->set_message("Invalid server type");
        return;
    }

    res->set_success(true);
    res->set_message("Server registered successfully");
}

void CentralServerImpl::Release_server_connection(
    rpc_server::ServerType server_type,
    const std::string& address,
    const std::string& port)
{
    switch (server_type)
    {
    case rpc_server::ServerType::BATTLE:
        battle_connection_pool_.remove_server(server_type, address, port);
        get_logger(poor::LogCategory::CONNECTION_POOL)->warn("Battle_connection_pool: unregistered {} {}", address, port);
        break;

    case rpc_server::ServerType::CENTRAL:
        central_connection_pool_.remove_server(server_type, address, port);
        get_logger(poor::LogCategory::CONNECTION_POOL)->warn("Central_connection_pool: unregistered {} {}", address, port);
        break;

    case rpc_server::ServerType::DB:
        db_connection_pool_.remove_server(server_type, address, port);
        get_logger(poor::LogCategory::CONNECTION_POOL)->warn("DB_connection_pool: unregistered {} {}", address, port);
        break;

    case rpc_server::ServerType::GATEWAY:
        gateway_connection_pool_.remove_server(server_type, address, port);
        get_logger(poor::LogCategory::CONNECTION_POOL)->warn("Gateway_connection_pool: unregistered {} {}", address, port);
        break;

    case rpc_server::ServerType::LOGIC:
        logic_connection_pool_.remove_server(server_type, address, port);
        get_logger(poor::LogCategory::CONNECTION_POOL)->warn("Logic_connection_pool: unregistered {} {}", address, port);
        break;

    case rpc_server::ServerType::LOGIN:
        login_connection_pool_.remove_server(server_type, address, port);
        get_logger(poor::LogCategory::CONNECTION_POOL)->warn("Login_connection_pool: unregistered {} {}", address, port);
        break;

    case rpc_server::ServerType::FILE:
        file_connection_pool_.remove_server(server_type, address, port);
        get_logger(poor::LogCategory::CONNECTION_POOL)->warn("File_connection_pool: unregistered {} {}", address, port);
        break;

    case rpc_server::ServerType::MATCHING:
        matching_connection_pool_.remove_server(server_type, address, port);
        get_logger(poor::LogCategory::CONNECTION_POOL)->warn("Matching_connection_pool: unregistered {} {}", address, port);
        break;
    }
}

void CentralServerImpl::All_connec_poor(
    const rpc_server::MultipleConnectPoorReq* req,
    rpc_server::MultipleConnectPoorRes* res)
{
    for (const auto& server_type : req->server_types())
    {
        rpc_server::ConnectPool* connect_pool = res->add_connect_pools();
        connect_pool->set_server_type(static_cast<rpc_server::ServerType>(server_type));

        std::unordered_map<rpc_server::ServerType, std::vector<std::pair<std::string, std::string>>> connections;

        switch (server_type)
        {
        case rpc_server::CENTRAL:
            connections = central_connection_pool_.get_all_connections();
            break;
        case rpc_server::DB:
            connections = db_connection_pool_.get_all_connections();
            break;
        case rpc_server::GATEWAY:
            connections = gateway_connection_pool_.get_all_connections();
            break;
        case rpc_server::LOGIC:
            connections = logic_connection_pool_.get_all_connections();
            break;
        case rpc_server::LOGIN:
            connections = login_connection_pool_.get_all_connections();
            break;
        case rpc_server::FILE:
            connections = file_connection_pool_.get_all_connections();
            break;
        case rpc_server::MATCHING:
            connections = matching_connection_pool_.get_all_connections();
            break;
        default:
            res->set_success(false);
            res->set_message("Invalid server type");
            return;
        }

        for (const auto& connection : connections[static_cast<rpc_server::ServerType>(server_type)])
        {
            rpc_server::ConnectInfo* conn_info = connect_pool->add_connect_info();
            conn_info->set_address(connection.first);
            conn_info->set_port(std::stoi(connection.second));
        }
    }

    res->set_success(true);
    res->set_message("Connection pools information retrieved successfully");
}

// ==================== 心跳检查 ====================

void CentralServerImpl::start_heartbeat_checker()
{
    if (heartbeat_checker_running_.load())
    {
        return;
    }

    heartbeat_checker_running_.store(true);
    heartbeat_checker_thread_ = std::thread(&CentralServerImpl::check_heartbeat_worker, this);
    
    log_activity("Heartbeat checker started");
}

void CentralServerImpl::stop_heartbeat_checker()
{
    if (!heartbeat_checker_running_.load())
    {
        return;
    }

    heartbeat_checker_running_.store(false);
    
    if (heartbeat_checker_thread_.joinable())
    {
        heartbeat_checker_thread_.join();
    }
    
    log_activity("Heartbeat checker stopped");
}

void CentralServerImpl::check_heartbeat_worker()
{
    while (heartbeat_checker_running_.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        std::lock_guard<std::mutex> lock(heartbeat_mutex_);
        auto now = std::chrono::steady_clock::now();

        for (const auto& record : heartbeat_records_)
        {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - record.second.last_heartbeat).count();

            if (elapsed > 30)
            {
                Release_server_connection(
                    record.second.server_type,
                    record.second.address,
                    record.second.port
                );

                get_logger(poor::LogCategory::HEARTBEAT)->warn(
                    "Server lost heartbeat: {} {}",
                    record.second.address,
                    record.second.port
                );
            }
        }
    }
}
