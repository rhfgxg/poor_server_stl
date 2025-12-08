#include "base_server.h"
#include "config_manager.h"
#include <lua.hpp>
#include <iostream>

// 构造函数
BaseServer::BaseServer(rpc_server::ServerType server_type, const std::string& server_address, const std::string& server_port, LoggerManager& logger_manager, size_t thread_count) :
    server_type_(server_type),
    server_address_(server_address),
    server_port_(server_port),
    running_(false),
    logger_manager_(logger_manager),
    thread_manager_(nullptr),
    central_connection_pool_(nullptr),
    heartbeat_running_(false),
    heartbeat_interval_(30)  // 默认 30 秒心跳间隔
{
    // 如果未指定线程数，使用硬件并发数
    if (thread_count == 0)
    {
        thread_count = std::thread::hardware_concurrency();
        if (thread_count == 0)
        {
            thread_count = 4;  // 如果无法检测，默认 4 个线程
        }
    }

    // 创建线程池管理器
    thread_manager_ = std::make_unique<ThreadManager>(thread_count);

    log_startup("BaseServer constructed with " + std::to_string(thread_count) + " threads");
}

// 析构函数
BaseServer::~BaseServer()
{
    if (running_.load())
    {
        stop();
    }
    log_shutdown("BaseServer destroyed");
}

// 启动服务器
bool BaseServer::start()
{
    if (running_.load())
    {
        log_activity("Server already running");
        return false;
    }

    log_startup("Starting server...");

    // 1. 启动线程池
    if (!init_thread_pool(0))
    {
        log_startup("Failed to initialize thread pool");
        return false;
    }
    log_startup("Thread pool initialized");

    // 2. 调用子类的初始化钩子
    if (!on_start())
    {
        log_startup("on_start() hook failed");
        stop_heartbeat();
        if (thread_manager_)
        {
            thread_manager_->stop();
        }
        return false;
    }
    log_startup("on_start() hook completed");

    // 3. 初始化到 Central Server 的连接（仅非 Central Server）
    if (server_type_ != rpc_server::ServerType::CENTRAL)
    {
        if (!init_central_connection())
        {
            log_startup("Failed to initialize Central Server connection");
            on_stop();
            if (thread_manager_)
            {
                thread_manager_->stop();
            }
            return false;
        }
        log_startup("Central Server connection initialized");

        // 4. 注册到 Central Server
        if (!register_to_central())
        {
            log_startup("Failed to register to Central Server");
            on_stop();
            if (thread_manager_)
            {
                thread_manager_->stop();
            }
            return false;
        }
        log_startup("Registered to Central Server");

        // 5. 启动心跳
        start_heartbeat();
        log_startup("Heartbeat started");
    }

    running_.store(true);
    log_startup("Server started successfully");
    return true;
}

// 停止服务器
void BaseServer::stop()
{
    if (!running_.load())
    {
        log_activity("Server already stopped");
        return;
    }

    log_shutdown("Stopping server...");

    running_.store(false);

    // 1. 调用子类的停止钩子
    on_stop();
    log_shutdown("on_stop() hook completed");

    // 2. 停止心跳
    if (server_type_ != rpc_server::ServerType::CENTRAL)
    {
        stop_heartbeat();
        log_shutdown("Heartbeat stopped");

        // 3. 从 Central Server 注销
        unregister_from_central();
        log_shutdown("Unregistered from Central Server");
    }

    // 4. 停止线程池
    if (thread_manager_)
    {
        thread_manager_->stop();
        log_shutdown("Thread pool stopped");
    }

    log_shutdown("Server stopped successfully");
}

// 提交无返回值的异步任务
void BaseServer::submit_task_void(std::function<void()> task)
{
    if (thread_manager_)
    {
        thread_manager_->enqueue(std::move(task));
    }
}

// 获取日志器
std::shared_ptr<spdlog::logger> BaseServer::get_logger(poor::LogCategory category)
{
    return logger_manager_.getLogger(category);
}

// 记录启动日志
void BaseServer::log_startup(const std::string& message)
{
    auto logger = get_logger(poor::LogCategory::STARTUP_SHUTDOWN);
    if (logger)
    {
        logger->info("[BaseServer] {}", message);
    }
}

// 记录关闭日志
void BaseServer::log_shutdown(const std::string& message)
{
    auto logger = get_logger(poor::LogCategory::STARTUP_SHUTDOWN);
    if (logger)
    {
        logger->info("[BaseServer] {}", message);
    }
}

// 记录心跳日志
void BaseServer::log_heartbeat(const std::string& message)
{
    auto logger = get_logger(poor::LogCategory::HEARTBEAT);
    if (logger)
    {
        logger->debug("[BaseServer] {}", message);
    }
}

// 记录应用活动日志
void BaseServer::log_activity(const std::string& message)
{
    auto logger = get_logger(poor::LogCategory::APPLICATION_ACTIVITY);
    if (logger)
    {
        logger->info("[BaseServer] {}", message);
    }
}

// ==================== 私有方法实现 ====================

// 初始化线程池
bool BaseServer::init_thread_pool(size_t thread_count [[maybe_unused]])
{
    if (!thread_manager_)
    {
        return false;
    }

    try
    {
        thread_manager_->start();
        return true;
    }
    catch (const std::exception& e)
    {
        log_startup(std::string("Thread pool initialization failed: ") + e.what());
        return false;
    }
}

// 初始化 Central Server 连接
bool BaseServer::init_central_connection()
{
    // 读取 Central Server 配置（自动尝试多个路径）
    if (!read_central_config(""))
    {
        log_startup("Failed to read Central Server config");
        return false;
    }

    // 创建到 Central Server 的连接池
    try
    {
        std::string central_target = central_server_address_ + ":" + central_server_port_;
        central_connection_pool_ = std::make_unique<ConnectionPool>(5);  // 5 个连接
        
        // 向连接池添加 Central Server
        central_connection_pool_->add_server(
            rpc_server::ServerType::CENTRAL,
            central_server_address_,
            central_server_port_
        );
        
        log_startup("Central Server connection pool created: " + central_target);
        return true;
    }
    catch (const std::exception& e)
    {
        log_startup(std::string("Failed to create Central connection pool: ") + e.what());
        return false;
    }
}

// 注册到 Central Server
bool BaseServer::register_to_central()
{
    if (!central_connection_pool_)
    {
        log_startup("Central connection pool not initialized");
        return false;
    }

    try
    {
        // 获取到 Central Server 的连接
        auto channel = central_connection_pool_->get_connection(rpc_server::ServerType::CENTRAL);
        if (!channel)
        {
            log_startup("Failed to get connection from Central pool");
            return false;
        }

        // 创建 stub
        auto stub = rpc_server::CentralServer::NewStub(channel);

        // 构造请求
        rpc_server::RegisterServerReq request;
        request.set_server_type(server_type_);
        request.set_address(server_address_);
        request.set_port(server_port_);

        // 发送请求
        rpc_server::RegisterServerRes response;
        grpc::ClientContext context;
        
        // 设置超时
        auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
        context.set_deadline(deadline);

        grpc::Status status = stub->Register_server(&context, request, &response);

        // 释放连接
        central_connection_pool_->release_connection(rpc_server::ServerType::CENTRAL, channel);

        if (status.ok() && response.success())
        {
            log_startup("Successfully registered to Central Server");
            on_registered(true);
            return true;
        }
        else
        {
            log_startup("Failed to register to Central Server: " + 
                       (status.ok() ? response.message() : status.error_message()));
            on_registered(false);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        log_startup(std::string("Exception during registration: ") + e.what());
        on_registered(false);
        return false;
    }
}

// 从 Central Server 注销
bool BaseServer::unregister_from_central()
{
    if (!central_connection_pool_)
    {
        return true;  // 没有连接池，认为已经注销
    }

    try
    {
        // 获取到 Central Server 的连接
        auto channel = central_connection_pool_->get_connection(rpc_server::ServerType::CENTRAL);
        if (!channel)
        {
            log_shutdown("Failed to get connection from Central pool");
            return false;
        }

        // 创建 stub
        auto stub = rpc_server::CentralServer::NewStub(channel);

        // 构造请求
        rpc_server::UnregisterServerReq request;
        request.set_server_type(server_type_);
        request.set_address(server_address_);
        request.set_port(server_port_);

        // 发送请求
        rpc_server::UnregisterServerRes response;
        grpc::ClientContext context;
        
        // 设置超时
        auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
        context.set_deadline(deadline);

        grpc::Status status = stub->Unregister_server(&context, request, &response);

        // 释放连接
        central_connection_pool_->release_connection(rpc_server::ServerType::CENTRAL, channel);

        if (status.ok() && response.success())
        {
            log_shutdown("Successfully unregistered from Central Server");
            on_unregistered(true);
            return true;
        }
        else
        {
            log_shutdown("Failed to unregister from Central Server: " + 
                        (status.ok() ? response.message() : status.error_message()));
            on_unregistered(false);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        log_shutdown(std::string("Exception during unregistration: ") + e.what());
        on_unregistered(false);
        return false;
    }
}

// 启动心跳线程
void BaseServer::start_heartbeat()
{
    if (heartbeat_running_.load())
    {
        log_heartbeat("Heartbeat already running");
        return;
    }

    heartbeat_running_.store(true);
    heartbeat_thread_ = std::thread(&BaseServer::heartbeat_thread_func, this);
    log_heartbeat("Heartbeat thread started");
}

// 停止心跳线程
void BaseServer::stop_heartbeat()
{
    if (!heartbeat_running_.load())
    {
        return;
    }

    heartbeat_running_.store(false);
    
    if (heartbeat_thread_.joinable())
    {
        heartbeat_thread_.join();
    }
    
    log_heartbeat("Heartbeat thread stopped");
}

// 心跳线程函数
void BaseServer::heartbeat_thread_func()
{
    log_heartbeat("Heartbeat thread started");

    while (heartbeat_running_.load())
    {
        // 发送心跳
        bool success = send_heartbeat();
        
        // 调用钩子
        on_heartbeat_sent(success);

        // 等待下一个心跳间隔
        for (int i = 0; i < heartbeat_interval_.count() && heartbeat_running_.load(); ++i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    log_heartbeat("Heartbeat thread exiting");
}

// 发送心跳到 Central Server
bool BaseServer::send_heartbeat()
{
    if (!central_connection_pool_)
    {
        log_heartbeat("Central connection pool not initialized");
        return false;
    }

    try
    {
        // 获取到 Central Server 的连接
        auto channel = central_connection_pool_->get_connection(rpc_server::ServerType::CENTRAL);
        if (!channel)
        {
            log_heartbeat("Failed to get connection from Central pool");
            return false;
        }

        // 创建 stub
        auto stub = rpc_server::CentralServer::NewStub(channel);

        // 构造请求
        rpc_server::HeartbeatReq request;
        request.set_server_type(server_type_);
        request.set_address(server_address_);
        request.set_port(server_port_);

        // 发送请求
        rpc_server::HeartbeatRes response;
        grpc::ClientContext context;
        
        // 设置超时
        auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(3);
        context.set_deadline(deadline);

        grpc::Status status = stub->Heartbeat(&context, request, &response);

        // 释放连接
        central_connection_pool_->release_connection(rpc_server::ServerType::CENTRAL, channel);

        if (status.ok() && response.success())
        {
            log_heartbeat("Heartbeat sent successfully");
            return true;
        }
        else
        {
            log_heartbeat("Heartbeat failed: " + 
                         (status.ok() ? response.message() : status.error_message()));
            return false;
        }
    }
    catch (const std::exception& e)
    {
        log_heartbeat(std::string("Exception during heartbeat: ") + e.what());
        return false;
    }
}

// 读取配置文件获取 Central Server 地址
bool BaseServer::read_central_config(const std::string& config_file [[maybe_unused]])
{
    ConfigManager cfg;
    std::vector<std::string> possible_paths = {
        "config/cfg_central_server.lua",
        "config/cpp/cfg_server/cfg_central_server.lua",
        "../config/cpp/cfg_server/cfg_central_server.lua",
        "../../config/cpp/cfg_server/cfg_central_server.lua",
        "../../../config/cpp/cfg_server/cfg_central_server.lua"
    };

    if (!config_file.empty())
    {
        possible_paths.insert(possible_paths.begin(), config_file);
    }

    auto loaded = cfg.load_from_paths(possible_paths);
    if (!loaded)
    {
        log_startup("Failed to load Central Server config from any expected location");
        return false;
    }

    auto host_opt = cfg.get_string("host");
    auto port_opt = cfg.get_string("port");
    if (!host_opt || !port_opt)
    {
        // 尝试数字类型的port
        if (!port_opt)
        {
            auto port_int = cfg.get_int("port");
            if (port_int)
            {
                port_opt = std::to_string(*port_int);
            }
        }

        if (!host_opt || !port_opt)
        {
            log_startup("Central config missing host or port");
            return false;
        }
    }

    central_server_address_ = *host_opt;
    central_server_port_ = *port_opt;

    log_startup("Central Server config loaded from: " + *loaded);
    log_startup("Central Server config: " + central_server_address_ + ":" + central_server_port_);
    return true;
}
