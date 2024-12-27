#include "login_server.h"
#include <future>

LoginServerImpl::LoginServerImpl(LoggerManager& logger_manager_):
    logger_manager(logger_manager_),	// 日志管理器
    central_stub(myproject::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))), // 链接中心服务器
	db_connection_pool(10) // 初始化数据库服务器连接池，设置连接池大小为10
{
    register_server(); // 注册服务器

    // 启动定时任务，
    // 定时向中心服务器获取最新的连接池状态
    std::thread(&LoginServerImpl::update_connection_pool,this).detach();
    // 定时向中心服务器发送心跳包
    std::thread(&LoginServerImpl::send_heartbeat,this).detach();
}

LoginServerImpl::~LoginServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    unregister_server(); // 注销服务器

    // 记录关闭日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("LoginServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

/************************************ 线程池工具函数 ****************************************************/
// 启动线程池
void LoginServerImpl::start_thread_pool(int num_threads)
{// 相关注释请参考 /src/central/src/central/central_server.cpp/start_thread_pool()
    for(int i = 0; i < num_threads; ++i)
    {
        thread_pool.emplace_back(&LoginServerImpl::worker_thread,this);   // 创建线程
    }
}

// 停止线程池
void LoginServerImpl::stop_thread_pool()
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
void LoginServerImpl::worker_thread()
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

/************************************ 连接池管理 ********************************************************/
// 注册服务器
void LoginServerImpl::register_server() 
{
    // 请求
    myproject::RegisterServerRequest request;
    request.set_server_type(myproject::ServerType::LOGIN);
    request.set_address("127.0.0.1");
    request.set_port("50053");

    // 响应
    myproject::RegisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->RegisterServer(&context, request, &response);

    if (status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Login server registered successfully: {} {}","localhost","50053");
		init_connection_pool(); // 初始化连接池
    }
    else 
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->error("Login server registration failed: {} {}","localhost","50053");
    }
}

// 注销服务器
void LoginServerImpl::unregister_server() 
{
    // 请求
    myproject::UnregisterServerRequest request;
    request.set_server_type(myproject::ServerType::LOGIN);
    request.set_address("localhost");
    request.set_port("50053");

    // 响应
    myproject::UnregisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->UnregisterServer(&context, request, &response);

    if (status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Login server unregistered successfully: {} {}","localhost","50053");
    }
    else
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->error("Login server unregistration failed: {} {}","localhost","50053");
    }
}

// 初始化连接池
void LoginServerImpl::init_connection_pool()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    myproject::ConnectPoorRequest request;
    request.set_server_type(myproject::ServerType::DATA);
    // 响应
    myproject::ConnectPoorResponse response;

    grpc::Status status = central_stub->GetConnectPoor(&context, request, &response);

    if (status.ok())
    {
        // 更新登录服务器连接池
        for (const auto& server_info : response.connect_info())
        {
            db_connection_pool.add_server(myproject::ServerType::DATA, server_info.address(), std::to_string(server_info.port()));
        }
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Connection pool updated successfully");
    }
    else
    {
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->error("Failed to retrieve connection pool information");
    }
}

/************************************ 定时任务 ********************************************************/
// 定时任务：更新连接池
void LoginServerImpl::update_connection_pool()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::minutes(5)); // 每5分钟更新一次连接池
        init_connection_pool();
    }
}

// 定时任务：发送心跳包
void LoginServerImpl::send_heartbeat()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒发送一次心跳包

        myproject::HeartbeatRequest request;
        myproject::HeartbeatResponse response;
        grpc::ClientContext context;

        request.set_server_type(myproject::ServerType::LOGIN);
        request.set_address("127.0.0.1"); // 设置服务器ip
        request.set_port("50053"); // 设置服务器端口

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

/************************************ gRPC服务接口实现 ******************************************************/
// 登录服务接口
grpc::Status LoginServerImpl::Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response)
{
    // 深拷贝请求和响应对象
    auto request_copy = std::make_shared<myproject::LoginRequest>(*request);
    auto response_copy = std::make_shared<myproject::LoginResponse>();
    // 创建 promise 和 future
    std::promise<void> task_promise;
    std::future<void> task_future = task_promise.get_future();

    {// 将任务加入任务队列
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁

        task_queue.push([this,request_copy,response_copy,&task_promise] {
            // 获取用户名和密码
            std::string username = request_copy->username(); // 从 request 对象中提取用户名和密码
            std::string password = request_copy->password();
	        // 构造查询条件
            std::map<std::string, std::string> query = { {"user_name", username}, {"user_password", password} }; 

	        std::string responses = login_("poor_users", "users", query); // 查询的数据库名，表名，查询条件

            if (responses == "Login successful")
            {
                response_copy->set_success(true);    // 设置响应对象 response 的 success 字段为 true
                response_copy->set_message("Login successful");
            }
            else 
            {
                response_copy->set_success(false);
                response_copy->set_message("Login failed");
            }
            task_promise.set_value();  // 设置 promise 的值，通知主线程任务完成
        });
        
    }
    queue_cv.notify_one();  // 通知线程池有新任务

    // 等待任务完成
    task_future.get();

    // 将响应结果复制回原响应对象
    *response = *response_copy;

    // 返回gRPC状态
    return grpc::Status::OK;
}

// 注册服务接口
grpc::Status LoginServerImpl::Register(grpc::ServerContext* context, const myproject::RegisterRequest* request, myproject::RegisterResponse* response)
{

    // 返回gRPC状态
    return grpc::Status::OK;
}

// 令牌验证服务接口
grpc::Status LoginServerImpl::Authenticate(grpc::ServerContext* context, const myproject::AuthenticateRequest* request, myproject::AuthenticateResponse* response)
{

    return grpc::Status::OK;
}

/************************************ gRPC服务接口工具函数 **************************************************/
// 登录服务
std::string LoginServerImpl::login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query)
{
    // 构造请求
    myproject::ReadRequest read_request;
    read_request.set_database(database); // 设置查询数据库
    read_request.set_table(table); // 设置查询表
    for (auto& it : query)
    {
        (*read_request.mutable_query())[it.first] = it.second; // 设置查询条件
    }

    // 构造响应
    myproject::ReadResponse read_response;
    grpc::ClientContext client_context; // 包含 RPC 调用的元数据和其他信息

    // 获取连接池中的连接
    auto channel = db_connection_pool.get_connection(myproject::ServerType::DATA);
    auto db_stub = myproject::DatabaseServer::NewStub(channel);

    // 向数据库服务器发送查询请求
    grpc::Status status = db_stub->Read(&client_context, read_request, &read_response);

    if (status.ok() && read_response.success())
    {
        std::cout << "Login successful" << std::endl;
        return "Login successful";
    }
    else
    {
        std::cout << "Login failed" << std::endl;
        return "Login failed";
    }
}

// 注册服务
std::string LoginServerImpl::register_(const std::string& database, const std::string& table, std::map<std::string, std::string> data)
{
	return "注册成功";
}

// 令牌验证服务
std::string LoginServerImpl::authenticate_(const std::string& token)
{
	return "验证成功";
}
