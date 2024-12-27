#include "gateway_server.h"

GatewayServerImpl::GatewayServerImpl(LoggerManager& logger_manager_):
    logger_manager(logger_manager_),  // 日志管理器   
    central_stub(myproject::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))), // 中心服务器存根
    login_connection_pool(10) // 初始化登录服务器连接池，设置连接池大小为10
{
    register_server();  // 注册服务器

    // 启动定时任务，
    // 定时向中心服务器获取最新的连接池状态
    std::thread(&GatewayServerImpl::update_connection_pool, this).detach();
    // 定时向中心服务器发送心跳包
    std::thread(&GatewayServerImpl::send_heartbeat, this).detach();
}

GatewayServerImpl::~GatewayServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    unregister_server(); // 注销服务器

    // 记录关闭日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("GatewayServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

/*************************************** 多线程工具函数 *****************************************************************/
// 启动线程池
void GatewayServerImpl::start_thread_pool(int num_threads)
{// 相关注释请参考 /src/central/src/central/central_server.cpp/start_thread_pool()
    for(int i = 0; i < num_threads; ++i)
    {
        thread_pool.emplace_back(&GatewayServerImpl::worker_thread,this);   // 创建线程
    }
}

// 停止线程池
void GatewayServerImpl::stop_thread_pool()
{// 相关注释请参考 /src/central/src/central/central_server.cpp/stop_thread_pool()
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop_threads = true;
    }

    queue_cv.notify_all();

    for(auto& thread : thread_pool)
    {
        if(thread.joinable())
        {
            thread.join();
        }
    }
    thread_pool.clear();

    std::queue<std::function<void()>> empty;
    std::swap(task_queue,empty);
}

// 线程池工作函数
void GatewayServerImpl::worker_thread()
{// 相关注释请参考 /src/central/src/central/central_server.cpp/worker_thread()
    while(true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            queue_cv.wait(lock,[this] {
                return !task_queue.empty() || stop_threads;
            });

            if(stop_threads && task_queue.empty())
            {
                return;
            }
            task = std::move(task_queue.front());
            task_queue.pop();
        }
        task();
    }
}

/*************************************** 连接池管理 *********************************************************/
// 服务器注册
void GatewayServerImpl::register_server()
{
    // 请求
    myproject::RegisterServerRequest request;
    request.set_server_type(myproject::ServerType::GATEWAY);
    request.set_address("127.0.0.1");
    request.set_port("50051");
    // 响应
    myproject::RegisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->RegisterServer(&context, request, &response);

    if(status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Gateway server registered successfully: {} {}", "localhost", "50051");
        init_connection_pool(); // 初始化连接池
    }
    else
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("ERROR：Gateway server registration failed: {} {}", "localhost", "50051");
    }
}

// 注销服务器
void GatewayServerImpl::unregister_server()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    myproject::UnregisterServerRequest request;
    request.set_server_type(myproject::ServerType::GATEWAY);
    request.set_address("localhost");
    request.set_port("50051");

    // 响应
    myproject::UnregisterServerResponse response;

    grpc::Status status = central_stub->UnregisterServer(&context, request, &response);

    if(status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Gateway server unregistered successfully: {} {}", "localhost", "50051");
    }
    else
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("ERROR：Gateway server unregistration failed: {} {}", "localhost", "50051");
    }
}

// 初始化/更新连接池
void GatewayServerImpl::init_connection_pool()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    myproject::ConnectPoorRequest request;
    request.set_server_type(myproject::ServerType::LOGIN);
    // 响应
    myproject::ConnectPoorResponse response;

    grpc::Status status = central_stub->GetConnectPoor(&context, request ,&response);

    if(status.ok())
    {
        // 更新登录服务器连接池
        for(const auto& server_info : response.connect_info())
        {
            login_connection_pool.add_server(myproject::ServerType::LOGIN,server_info.address(),std::to_string(server_info.port()));
        }
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Gateway server updated connection pool successfully");
    }
    else
    {
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->error("Failed to get connection pool information");
    }
}

/******************************************* 定时任务 *****************************************************/
// 定时任务：更新连接池
void GatewayServerImpl::update_connection_pool()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::minutes(5)); // 每5分钟更新一次连接池
        init_connection_pool();
    }
}

// 定时任务：发送心跳包
void GatewayServerImpl::send_heartbeat()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒发送一次心跳包

        myproject::HeartbeatRequest request;
        myproject::HeartbeatResponse response;
        grpc::ClientContext context;

        request.set_server_type(myproject::ServerType::GATEWAY);
        request.set_address("127.0.0.1"); // 设置服务器ip
        request.set_port("50051"); // 设置服务器端口

        grpc::Status status = central_stub->Heartbeat(&context,request,&response);

        if(status.ok() && response.success())
        {
            logger_manager.getLogger(LogCategory::HEARTBEAT)->info("Heartbeat sent successfully.");
        }
        else
        {
            logger_manager.getLogger(LogCategory::HEARTBEAT)->error("Failed to send heartbeat.");
        }
    }
}

/**************************************** grpc服务接口定义 **************************************************************************/
// 服务转发接口
grpc::Status GatewayServerImpl::RequestForward(grpc::ServerContext* context,const myproject::ForwardRequest* request,myproject::ForwardResponse* response)
{
    // 深拷贝请求和响应对象
    auto request_copy = std::make_shared<myproject::ForwardRequest>(*request);
    auto response_copy = std::make_shared<myproject::ForwardResponse>();
    // 创建 promise 和 future
    std::promise<void> task_promise;
    std::future<void> task_future = task_promise.get_future();

    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this,request_copy,response_copy,&task_promise] {
            switch(request_copy->service_type()) // 根据请求的服务类型进行转发
            {
            case myproject::ServiceType::REQ_LOGIN: // 用户登录请求
            {
                forward_to_login_service(request_copy->payload(),response_copy.get());  // 解析负载，并转发到登录服务
                break;
            }
            default:    // 未知服务类型
            {
                response_copy->set_success(false);
                break;
            }
            }
            task_promise.set_value();  // 设置 promise 的值，通知主线程任务完成
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务

    // 等待任务完成
    task_future.wait();

    // 将响应结果复制回原响应对象
    *response = *response_copy;

    return grpc::Status::OK;
}

/**************************************** grpc服务接口工具函数 **************************************************************************/
// Login 方法，处理登录请求
grpc::Status GatewayServerImpl::forward_to_login_service(const std::string& payload, myproject::ForwardResponse* response)
{
    myproject::LoginRequest login_request;  // 创建登录请求对象
    bool request_out = login_request.ParseFromString(payload); // 将负载解析为登录请求对象

    if(!request_out) // 如果解析失败
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,"Failed to parse LoginRequest");
    }

    // 构造响应
    myproject::LoginResponse login_response;
    grpc::ClientContext context;

    // 获取连接池中的连接
    auto channel = login_connection_pool.get_connection(myproject::ServerType::LOGIN);
    auto login_stub = myproject::LoginServer::NewStub(channel);

    grpc::Status status = login_stub->Login(&context,login_request,&login_response);

    if(!status.ok()) // 如果调用失败
    {
        return status;
    }

    bool response_out = login_response.SerializeToString(response->mutable_response()); // 将登录响应序列化为转发响应
    if(!response_out) // 如果序列化失败
    {
        return grpc::Status(grpc::StatusCode::INTERNAL,"Failed to serialize LoginResponse");
    }

    if(login_response.success()) // 如果登录成功
    {
        response->set_success(true);    // 设置响应对象 response 的 success 字段为 true
    }
    else
    {
        response->set_success(false);
    }

    return grpc::Status::OK;
}
