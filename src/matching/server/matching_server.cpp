#include "matching_server.h"

MatchingServerImpl::MatchingServerImpl(LoggerManager& logger_manager_, const std::string address, std::string port):
    server_type(rpc_server::ServerType::MATCHING),
    server_address(address),
    server_port(port),
    logger_manager(logger_manager_),
    redis_client(),
    central_stub(rpc_server::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))),
    db_connection_pool(10)
{
    redis_client.get_client()->connect("127.0.0.1", 6379); // 连接Redis服务器
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("redis connection successful");

    register_server(); // 注册服务器

    // 启动定时任务，
    // 定时向中心服务器获取最新的连接池状态
    std::thread(&MatchingServerImpl::Update_connection_pool, this).detach();
    // 定时向中心服务器发送心跳包
    std::thread(&MatchingServerImpl::Send_heartbeat, this).detach();
}

MatchingServerImpl::~MatchingServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    unregister_server(); // 注销服务器

    // 记录关闭日志
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("MatchingServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

/************************************ 线程池工具函数 ****************************************************/
// 启动线程池
void MatchingServerImpl::start_thread_pool(int num_threads)
{// 相关注释请参考 /src/central/src/central/central_server.cpp/start_thread_pool()
    for(int i = 0; i < num_threads; ++i)
    {
        this->thread_pool.emplace_back(&MatchingServerImpl::Worker_thread, this);   // 创建线程
    }
}

// 停止线程池
void MatchingServerImpl::stop_thread_pool()
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
    std::swap(this->task_queue, empty);
}

// 添加异步任务
std::future<void> MatchingServerImpl::add_async_task(std::function<void()> task)
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
void MatchingServerImpl::Worker_thread()
{// 相关注释请参考 /src/central/src/central/central_server.cpp/worker_thread()
    while(true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);

            this->queue_cv.wait(lock, [this] {
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

/************************************ 连接池管理 ********************************************************/
// 注册服务器
void MatchingServerImpl::register_server()
{
    // 请求
    rpc_server::RegisterServerReq request;
    request.set_server_type(this->server_type);
    request.set_address(this->server_address);
    request.set_port(this->server_port);

    // 响应
    rpc_server::RegisterServerRes response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->Register_server(&context, request, &response);

    if(status.ok() && response.success())
    {
        logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Matching server registered successfully: {} {}", this->server_address, this->server_port);
        Init_connection_pool(); // 初始化连接池
    } else
    {
        logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->error("Matching server registration failed: {} {}", this->server_address, this->server_port);
    }
}

// 注销服务器
void MatchingServerImpl::unregister_server()
{
    // 请求
    rpc_server::UnregisterServerReq request;
    request.set_server_type(this->server_type);
    request.set_address(this->server_address);
    request.set_port(this->server_port);

    // 响应
    rpc_server::UnregisterServerRes response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = this->central_stub->Unregister_server(&context, request, &response);

    if(status.ok() && response.success())
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Matching server unregistered successfully: {} {}", this->server_address, this->server_port);
    }
    else
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->error("Matching server unregistration failed: {} {}", this->server_address, this->server_port);
    }
}

// 初始化连接池
void MatchingServerImpl::Init_connection_pool()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    rpc_server::MultipleConnectPoorReq req;
    req.add_server_types(rpc_server::ServerType::LOGIC);
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
                case rpc_server::ServerType::LOGIC:
                {
                    db_connection_pool.add_server(rpc_server::ServerType::LOGIC, conn_info.address(), std::to_string(conn_info.port()));
                    break;
                }
                default:
                break;
                }
            }
        }
        logger_manager.getLogger(poor::LogCategory::CONNECTION_POOL)->info("Battle server updated connection pools successfully");
    }
    else
    {
        logger_manager.getLogger(poor::LogCategory::CONNECTION_POOL)->error("Failed to get connection pools information");
    }
}

/************************************ 定时任务 ********************************************************/
// 定时任务：更新连接池
void MatchingServerImpl::Update_connection_pool()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::minutes(5)); // 每5分钟更新一次连接池
        this->Init_connection_pool();
    }
}

// 定时任务：发送心跳包
void MatchingServerImpl::Send_heartbeat()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒发送一次心跳包

        rpc_server::HeartbeatReq request;
        rpc_server::HeartbeatRes response;
        grpc::ClientContext context;

        request.set_server_type(this->server_type);
        request.set_address(this->server_address);
        request.set_port(this->server_port);

        grpc::Status status = central_stub->Heartbeat(&context, request, &response);

        if(status.ok() && response.success())
        {
            this->logger_manager.getLogger(poor::LogCategory::HEARTBEAT)->info("Heartbeat sent successfully.");
        }
        else
        {
            this->logger_manager.getLogger(poor::LogCategory::HEARTBEAT)->error("Failed to send heartbeat.");
        }
    }
}

/************************************ gRPC服务接口实现 ******************************************************/

/************************************ gRPC服务接口工具函数 **************************************************/

/******************************************** 其他工具函数 ***********************************************/
