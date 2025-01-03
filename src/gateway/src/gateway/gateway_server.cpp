#include "gateway_server.h"

GatewayServerImpl::GatewayServerImpl(LoggerManager& logger_manager_):
    server_type(rpc_server::ServerType::GATEWAY),    // 服务器类型
    logger_manager(logger_manager_),  // 日志管理器   
    central_stub(rpc_server::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))), // 中心服务器存根
    login_connection_pool(10) // 初始化登录服务器连接池，设置连接池大小为10
{
    
    this->Read_server_config();   // 读取配置文件并初始化服务器地址和端口

    this->register_server();  // 注册服务器

    // 启动定时任务，
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
    request.set_address("127.0.0.1");
    request.set_port("50051");
    // 响应
    rpc_server::RegisterServerRes response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = this->central_stub->Register_server(&context, request, &response);

    if(status.ok() && response.success())
    {
        this->logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Gateway server registered successfully: {} {}", "localhost", "50051");
        this->Init_connection_pool(); // 初始化连接池
    }
    else
    {
        this->logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("ERROR：Gateway server registration failed: {} {}", "localhost", "50051");
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
    request.set_address("localhost");
    request.set_port("50051");

    // 响应
    rpc_server::UnregisterServerRes response;

    grpc::Status status = this->central_stub->Unregister_server(&context, request, &response);

    if(status.ok() && response.success())
    {
        this->logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Gateway server unregistered successfully: {} {}", "localhost", "50051");
    }
    else
    {
        this->logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("ERROR：Gateway server unregistration failed: {} {}", "localhost", "50051");
    }
}

// 初始化/更新连接池
void GatewayServerImpl::Init_connection_pool()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    rpc_server::ConnectPoorReq request;
    request.set_server_type(rpc_server::ServerType::LOGIN);
    // 响应
    rpc_server::ConnectPoorRes response;

    grpc::Status status = central_stub->Get_connec_poor(&context, request ,&response);

    if(status.ok())
    {
        // 更新登录服务器连接池
        for(const auto& server_info : response.connect_info())
        {
            login_connection_pool.add_server(rpc_server::ServerType::LOGIN,server_info.address(),std::to_string(server_info.port()));
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
grpc::Status GatewayServerImpl::Request_forward(grpc::ServerContext* context, const rpc_server::ForwardReq* req, rpc_server::ForwardRes* res)
{
    auto task_future = this->add_async_task([this,req,res] {
        switch(req->service_type()) // 根据请求的服务类型进行转发
        {
        case rpc_server::ServiceType::REQ_LOGIN: // 用户登录请求
        {
            Forward_to_login_service(req->payload(), res);  // 解析负载，并转发到登录服务
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

/**************************************** grpc服务接口工具函数 **************************************************************************/
// Login 方法，处理登录请求
grpc::Status GatewayServerImpl::Forward_to_login_service(const std::string& payload, rpc_server::ForwardRes* response)
{
    rpc_server::LoginReq login_request;  // 创建登录请求对象
    bool request_out = login_request.ParseFromString(payload); // 将负载解析为登录请求对象

    if(!request_out) // 如果解析失败
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,"Failed to parse LoginRequest");
    }

    // 构造响应
    rpc_server::LoginRes login_response;
    grpc::ClientContext context;

    // 获取连接池中的连接
    auto channel = this->login_connection_pool.get_connection(rpc_server::ServerType::LOGIN);
    auto login_stub = rpc_server::LoginServer::NewStub(channel);

    grpc::Status status = login_stub->Login(&context, login_request, &login_response);

    if(!status.ok()) // 如果调用失败
    {
        return status;
    }

    bool response_out = login_response.SerializeToString(response->mutable_response()); // 将登录响应序列化为转发响应
    if(!response_out) // 如果序列化失败
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to serialize LoginResponse");
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

/******************************************** 其他工具函数 ***********************************************/
// 读取服务器配置文件，初始化服务器地址和端口
void GatewayServerImpl::Read_server_config()
{
    lua_State* L = luaL_newstate();  // 创建lua虚拟机
    luaL_openlibs(L);   // 打开lua标准库

    std::string file_url = "config/gateway_server_config.lua";  // 配置文件路径

    if(luaL_dofile(L,file_url.c_str()) != LUA_OK)
    {
        lua_close(L);
        throw std::runtime_error("Failed to load config file");
    }

    lua_getglobal(L,"gateway_server");
    if(!lua_istable(L,-1))
    {
        lua_close(L);
        throw std::runtime_error("Invalid config format");
    }

    lua_getfield(L,-1,"host");
    if(!lua_isstring(L,-1))
    {
        lua_close(L);
        throw std::runtime_error("Invalid host format");
    }
    this->server_address = lua_tostring(L,-1);
    lua_pop(L,1);

    lua_getfield(L,-1,"port");
    if(!lua_isinteger(L,-1))
    {
        lua_close(L);
        throw std::runtime_error("Invalid port format");
    }
    this->server_port = std::to_string(lua_tointeger(L,-1));
    lua_pop(L,1);

    lua_close(L);   // 关闭lua虚拟机
}
