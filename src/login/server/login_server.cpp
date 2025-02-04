#include "login_server.h"

LoginServerImpl::LoginServerImpl(LoggerManager& logger_manager_, const std::string address, std::string port):
    server_type(rpc_server::ServerType::LOGIN),
    server_address(address),
    server_port(port),
    logger_manager(logger_manager_),
    redis_client(),
    central_stub(rpc_server::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))),
    db_connection_pool(10)
{
    redis_client.get_client()->connect("127.0.0.1", 6379); // 连接Redis服务器
    logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->info("redis connection successful");

    register_server(); // 注册服务器

    // 启动定时任务，
    // 定时向中心服务器获取最新的连接池状态
    std::thread(&LoginServerImpl::Update_connection_pool,this).detach();
    // 定时向中心服务器发送心跳包
    std::thread(&LoginServerImpl::Send_heartbeat,this).detach();
}

LoginServerImpl::~LoginServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    unregister_server(); // 注销服务器

    // 记录关闭日志
    logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->info("LoginServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

/************************************ 线程池工具函数 ****************************************************/
// 启动线程池
void LoginServerImpl::start_thread_pool(int num_threads)
{// 相关注释请参考 /src/central/src/central/central_server.cpp/start_thread_pool()
    for(int i = 0; i < num_threads; ++i)
    {
        this->thread_pool.emplace_back(&LoginServerImpl::Worker_thread, this);   // 创建线程
    }
}

// 停止线程池
void LoginServerImpl::stop_thread_pool()
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
std::future<void> LoginServerImpl::add_async_task(std::function<void()> task)
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
void LoginServerImpl::Worker_thread()
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

/************************************ 连接池管理 ********************************************************/
// 注册服务器
void LoginServerImpl::register_server() 
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

    if (status.ok() && response.success())
    {
        logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->info("Login server registered successfully: {} {}", this->server_address, this->server_port);
        Init_connection_pool(); // 初始化连接池
    }
    else 
    {
        logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->error("Login server registration failed: {} {}", this->server_address, this->server_port);
    }
}

// 注销服务器
void LoginServerImpl::unregister_server() 
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

    if (status.ok() && response.success())
    {
        this->logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->info("Login server unregistered successfully: {} {}", this->server_address, this->server_port);
    }
    else
    {
        this->logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->error("Login server unregistration failed: {} {}", this->server_address, this->server_port);
    }
}

// 初始化连接池
void LoginServerImpl::Init_connection_pool()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    rpc_server::MultipleConnectPoorReq req;
    req.add_server_types(rpc_server::ServerType::DB);
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
                case rpc_server::ServerType::DB:
                {
                    db_connection_pool.add_server(rpc_server::ServerType::DB, conn_info.address(), std::to_string(conn_info.port()));
                    break;
                }
                default:
                break;
                }
            }
        }
        logger_manager.getLogger(rpc_server::LogCategory::CONNECTION_POOL)->info("Login server updated connection pools successfully");
    }
    else
    {
        logger_manager.getLogger(rpc_server::LogCategory::CONNECTION_POOL)->error("Failed to get connection pools information");
    }
}

/************************************ 定时任务 ********************************************************/
// 定时任务：更新连接池
void LoginServerImpl::Update_connection_pool()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::minutes(5)); // 每5分钟更新一次连接池
        this->Init_connection_pool();
    }
}

// 定时任务：发送心跳包
void LoginServerImpl::Send_heartbeat()
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

        grpc::Status status = central_stub->Heartbeat(&context,request,&response);

        if(status.ok() && response.success())
        {
            this->logger_manager.getLogger(rpc_server::LogCategory::HEARTBEAT)->info("Heartbeat sent successfully.");
        }
        else
        {
            this->logger_manager.getLogger(rpc_server::LogCategory::HEARTBEAT)->error("Failed to send heartbeat.");
        }
    }
}

/************************************ gRPC服务接口实现 ******************************************************/
// 登录服务接口
grpc::Status LoginServerImpl::Login(grpc::ServerContext* context, const rpc_server::LoginReq* req, rpc_server::LoginRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_login(req, res); // 查询的数据库名，表名，查询条件
    });

    // 等待任务完成
    task_future.get();

    return grpc::Status::OK;
}

// 注册服务接口
grpc::Status LoginServerImpl::Register(grpc::ServerContext* context, const rpc_server::RegisterReq* req, rpc_server::RegisterRes* res)
{

    // 返回gRPC状态
    return grpc::Status::OK;
}

// 令牌验证服务接口
grpc::Status LoginServerImpl::Authenticate(grpc::ServerContext* context, const rpc_server::AuthenticateReq* req, rpc_server::AuthenticateRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_authenticate(req, res); // 查询的数据库名，表名，查询条件
    });

    // 等待任务完成
    task_future.get();

    return grpc::Status::OK;
}

/************************************ gRPC服务接口工具函数 **************************************************/
// 登录服务
void LoginServerImpl::Handle_login(const rpc_server::LoginReq* req, rpc_server::LoginRes* res)
{
    // 获取用户名和密码
    std::string account = req->account();
    std::string password = req->password();

    // 构造查询条件
    std::map<std::string, std::string> query = {{"user_account", account}, {"user_password", password}};

    // 构造数据库查询请求
    rpc_server::ReadReq read_request;
    read_request.set_database("poor_users");
    read_request.set_table("users");
    for(auto& it : query)
    {
        (*read_request.mutable_query())[it.first] = it.second;
    }

    // 构造响应与客户端上下文
    rpc_server::ReadRes read_response;
    grpc::ClientContext client_context;

    // 从连接池中获取数据库服务器连接
    auto channel = this->db_connection_pool.get_connection(rpc_server::ServerType::DB);
    auto db_stub = rpc_server::DBServer::NewStub(channel);

    // 调用数据库服务器的查询服务
    grpc::Status status = db_stub->Read(&client_context, read_request, &read_response);

    if(status.ok() && read_response.success())
    {
        // 生成Token
        std::string token = GenerateToken(account);

        // 将用户在线状态存储到Redis中
        auto client = redis_client.get_client();
        client->set(account, "online");
        client->expire(account, 1800); // 设置30分钟过期时间
        client->set(account + "_token", token);
        client->expire(account + "_token", 1800);

        res->set_success(true);
        res->set_message("Login successful");
        res->set_token(token);
        this->logger_manager.getLogger(rpc_server::LogCategory::APPLICATION_ACTIVITY)->info("Login successful");
    }
    else
    {
        res->set_success(false);
        res->set_message("Login failed");
        this->logger_manager.getLogger(rpc_server::LogCategory::APPLICATION_ACTIVITY)->info("Login failed");
    }

    this->db_connection_pool.release_connection(rpc_server::ServerType::DB, channel); // 释放数据库服务器连接
}

// 注册服务
void LoginServerImpl::Handle_register(const rpc_server::RegisterReq* req,rpc_server::RegisterRes* res)    // 注册
{
}

// 令牌验证服务
void LoginServerImpl::Handle_authenticate(const rpc_server::AuthenticateReq* req, rpc_server::AuthenticateRes* res)
{
    // 获取Token和账号
    std::string token = req->token();
    std::string account = req->account();

    // 验证Token的有效性
    if(!ValidateToken(token, account))
    {
        res->set_success(false);
        res->set_message("Invalid token");
        return;
    }

    // 检查用户的在线状态
    auto client = redis_client.get_client();
    auto reply = client->exists({account});
    client->sync_commit();
    if(reply.get().as_integer() == 1)
    {
        // 延续Token的有效期
        client->expire(account, 1800);
        client->expire(account + "_token", 1800);

        res->set_success(true);
        res->set_message("Token validated");
    }
    else
    {
        res->set_success(false);
        res->set_message("User is not online");
    }
}
/******************************************** 其他工具函数 ***********************************************/
// 生成 用户token
std::string LoginServerImpl::GenerateToken(const std::string& account)
{
    auto token = jwt::create()
        .set_issuer("auth0")
        .set_type("JWS")
        .set_subject(account)
        .set_audience("example.com")
        .set_issued_at(std::chrono::system_clock::now())    // 不设置过期时间，依靠Redis的过期时间
        .sign(jwt::algorithm::hs256{"secret"});

    return token;
}

// 验证 token
bool LoginServerImpl::ValidateToken(const std::string& token, const std::string& account)
{
    try
    {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{"secret"})
            .with_issuer("auth0")
            .with_subject(account);

        verifier.verify(decoded);
        return true;
    }
    catch(const std::exception& e)
    {
        return false;
    }
}
