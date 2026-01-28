#include "central_server.h"
#include <string>

// 构造函数 - 继承 BaseServer
CentralServerImpl::CentralServerImpl(LoggerManager& logger_manager_, const std::string& address, const std::string& port):
    BaseServer(rpc_server::ServerType::CENTRAL, address, port, logger_manager_, 4),  // 调用基类构造函数，4个线程
    heartbeat_checker_running_(false),
    central_connection_pool_(10),
    db_connection_pool_(10),
    redis_connection_pool_(10),
    file_connection_pool_(10),
    gateway_connection_pool_(10),
    login_connection_pool_(10),
{
    // 连接池初始化日志
    log_activity("Central_connection_pool: initialized, size: 10");
    log_activity("DB_connection_pool: initialized, size: 10");
    log_activity("Redis_connection_pool: initialized, size: 10");
    log_activity("Gateway_connection_pool: initialized, size: 10");
    log_activity("Login_connection_pool: initialized, size: 10");
    log_activity("File_connection_pool: initialized, size: 10");

    // 将本服务器加入中心连接池
    central_connection_pool_.add_server(rpc_server::ServerType::CENTRAL, get_server_address(), get_server_port());
    log_activity("Central server added to connection pool: " + get_server_address() + ":" + get_server_port());
}

// 析构函数
CentralServerImpl::~CentralServerImpl()
{
    stop_heartbeat_checker();
    log_shutdown("Central_server stopped");
}

// ==================== BaseServer 钩子方法 ====================
// 服务器启动时调用
bool CentralServerImpl::on_start()
{
    log_startup("CentralServer initializing...");
    
    // 启动心跳检查线程
    start_heartbeat_checker();
    
    log_startup("CentralServer initialized successfully");
    return true;
}

// 服务器停止时调用
void CentralServerImpl::on_stop()
{
    log_shutdown("CentralServer shutting down...");
    
    // 停止心跳检查线程
    stop_heartbeat_checker();
    
    log_shutdown("CentralServer shutdown complete");
}

// ==================== gRPC 服务接口 ====================
// 服务器注册
grpc::Status CentralServerImpl::Register_server(grpc::ServerContext* context, const rpc_server::RegisterServerReq* req, rpc_server::RegisterServerRes* res)
{
    // 使用 BaseServer 的 submit_task 提交异步任务
    auto task_future = submit_task([this, req, res]() {
        Handle_register_server(req, res);
    });

    task_future.get();
    return grpc::Status::OK;
}

// 服务器断开
grpc::Status CentralServerImpl::Unregister_server(grpc::ServerContext* context, const rpc_server::UnregisterServerReq* req, rpc_server::UnregisterServerRes* res)
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

// 获取连接池中所有链接
grpc::Status CentralServerImpl::Get_connec_poor(grpc::ServerContext* context, const rpc_server::MultipleConnectPoorReq* req, rpc_server::MultipleConnectPoorRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        All_connec_poor(req, res);
    });

    task_future.get();
    return grpc::Status::OK;
}

// 接收心跳包
grpc::Status CentralServerImpl::Heartbeat(grpc::ServerContext* context, const rpc_server::HeartbeatReq* req, rpc_server::HeartbeatRes* res)
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
// 执行注册服务器
void CentralServerImpl::Handle_register_server(const rpc_server::RegisterServerReq* req, rpc_server::RegisterServerRes* res)
{
    rpc_server::ServerType server_type = req->server_type();
    std::string address = req->address();
    std::string port = req->port();

    auto* pool = get_connection_pool(server_type);
    if (!pool)
    {
        res->set_success(false);
        res->set_message("Invalid server type");
        return;
    }

    pool->add_server(server_type, address, port);
    log_activity(std::string(get_pool_label(server_type)) + ": registered " + address + ":" + port);

    res->set_success(true);
    res->set_message("Server registered successfully");
}

// 释放连接池中服务器连接
void CentralServerImpl::Release_server_connection(rpc_server::ServerType server_type, const std::string& address, const std::string& port)
{
    auto* pool = get_connection_pool(server_type);
    if (!pool)
    {
        return;
    }

    pool->remove_server(server_type, address, port);
    get_logger(poor::LogCategory::CONNECTION_POOL)->warn("{}: unregistered {} {}",
        get_pool_label(server_type), address, port);
}

// 返回连接池中的连接
void CentralServerImpl::All_connec_poor(const rpc_server::MultipleConnectPoorReq* req, rpc_server::MultipleConnectPoorRes* res)
{
    for (const auto& type_value : req->server_types())
    {
        auto server_type = static_cast<rpc_server::ServerType>(type_value);
        auto* pool = get_connection_pool(server_type);
        if (!pool)
        {
            res->set_success(false);
            res->set_message("Invalid server type");
            return;
        }

        rpc_server::ConnectPool* connect_pool = res->add_connect_pools();
        connect_pool->set_server_type(server_type);

        auto connections = pool->get_all_connections();
        auto it = connections.find(server_type);
        if (it == connections.end())
        {
            continue;
        }

        for (const auto& connection : it->second)
        {
            rpc_server::ConnectInfo* conn_info = connect_pool->add_connect_info();
            conn_info->set_address(connection.first);
            conn_info->set_port(std::stoi(connection.second));
        }
    }

    res->set_success(true);
    res->set_message("Connection pools information retrieved successfully");
}

// 获取对应类型的连接池
ConnectionPool* CentralServerImpl::get_connection_pool(rpc_server::ServerType server_type)
{
    switch (server_type)
    {
    case rpc_server::ServerType::CENTRAL:
        return &central_connection_pool_;
    case rpc_server::ServerType::DB:
        return &db_connection_pool_;
    case rpc_server::ServerType::REDIS:
        return &redis_connection_pool_;
    case rpc_server::ServerType::FILE:
        return &file_connection_pool_;
    case rpc_server::ServerType::GATEWAY:
        return &gateway_connection_pool_;
    case rpc_server::ServerType::LOGIN:
        return &login_connection_pool_;
    default:
        return nullptr;
    }
}

// 获取连接池标签
const char* CentralServerImpl::get_pool_label(rpc_server::ServerType server_type) const
{
    switch (server_type)
    {
    case rpc_server::ServerType::CENTRAL:
        return "Central_connection_pool";
    case rpc_server::ServerType::DB:
        return "DB_connection_pool";
    case rpc_server::ServerType::REDIS:
        return "Redis_connection_pool";
    case rpc_server::ServerType::FILE:
        return "File_connection_pool";
    case rpc_server::ServerType::GATEWAY:
        return "Gateway_connection_pool";
    case rpc_server::ServerType::LOGIN:
        return "Login_connection_pool";
    default:
        return "Unknown_connection_pool";
    }
}

// 启动心跳检查
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

// 停止心跳检查
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

// 心跳检查工作线程
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
                    record.second.port);

                get_logger(poor::LogCategory::HEARTBEAT)->warn(
                    "Server lost heartbeat: {} {}",
                    record.second.address,
                    record.second.port);
            }
        }
    }
}
