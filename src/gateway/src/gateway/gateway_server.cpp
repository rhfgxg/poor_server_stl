#include "gateway_server.h"

GatewayServerImpl::GatewayServerImpl(LoggerManager& logger_manager_):
    logger_manager(logger_manager_),  // 日志管理器   
    central_stub(myproject::CentralServer::NewStub(grpc::CreateChannel("localhost:50050",grpc::InsecureChannelCredentials()))), // 中心服务器存根
    login_connection_pool(10) // 初始化登录服务器连接池，设置连接池大小为10
{
    register_server();  // 注册服务器
    // 启动定时任务，定时向中心服务器获取最新的连接池状态
}

// GatewayServerImpl 析构函数
GatewayServerImpl::~GatewayServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    unregister_server(); // 注销服务器

    // 记录关闭日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("GatewayServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

// 启动线程池
void GatewayServerImpl::start_thread_pool(int num_threads)
{
    for(int i = 0; i < num_threads; ++i)
    {
        thread_pool.emplace_back(&GatewayServerImpl::worker_thread,this);   // 创建线程
    }
}

// 停止线程池
void GatewayServerImpl::stop_thread_pool()
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        stop_threads = true;    // 标志设置为 true，通知它应该停止工作
    }

    queue_cv.notify_all();  // 通知所有线程，检查 stop_threads 标志

    for(auto& thread : thread_pool) // 遍历线程池
    {
        if(thread.joinable())   // 如果线程可加入
        {
            thread.join();  // 加入线程
        }
    }
    thread_pool.clear();  // 清空线程池
    // 清空任务队列
    std::queue<std::function<void()>> empty;
    std::swap(task_queue,empty);
}

// 线程池工作函数
void GatewayServerImpl::worker_thread()
{
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

    grpc::Status status = central_stub->RegisterServer(&context,request,&response);

    if(status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("网关服务器注册成功: {} {}","localhost","50051");
        init_connection_pool(); // 初始化连接池
    }
    else
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("网关服务器注册失败: {} {}","localhost","50051");
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

    grpc::Status status = central_stub->UnregisterServer(&context,request,&response);

    if(status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("网关服务器注销成功: {} {}","localhost","50051");
    }
    else
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("网关服务器注销失败: {} {}","localhost","50051");
    }
}

// 初始化连接池
void GatewayServerImpl::init_connection_pool()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    myproject::ConnectPoorRequest request;
    request.set_server_type(myproject::ServerType::LOGIN);
    // 响应
    myproject::ConnectPoorResponse response;

    grpc::Status status = central_stub->GetConnectPoor(&context,request,&response);

    if(status.ok())
    {
        // 更新登录服务器连接池
        for(const auto& server_info : response.connect_info())
        {
            login_connection_pool.add_server(myproject::ServerType::LOGIN,server_info.address(),std::to_string(server_info.port()));
        }
    }
    else
    {
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("无法获取连接池信息");
    }
}

// 服务转发接口
grpc::Status GatewayServerImpl::RequestForward(grpc::ServerContext* context,const myproject::ForwardRequest* request,myproject::ForwardResponse* response)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this,request,response] {
            switch(request->service_type()) // 根据请求的服务类型进行转发
            {
            case myproject::ServiceType::REQ_LOGIN: // 用户登录请求
            {
                forward_to_login_service(request->payload(),response);
                break;
            }
            default:    // 未知服务类型
            response->set_success(false);
            break;
            }
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务
    return grpc::Status::OK;
}

// Login 方法，处理登录请求
grpc::Status GatewayServerImpl::forward_to_login_service(const std::string& payload,myproject::ForwardResponse* response)
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

    response->set_success(true);
    return grpc::Status::OK;
}

