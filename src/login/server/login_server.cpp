#include "login_server.h"
#include <chrono>
#include <iomanip>
#include <sstream>

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
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("redis connection successful");

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
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("LoginServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

/************************************ 线程池函数 ****************************************************/
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
        logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Login server registered successfully: {} {}", this->server_address, this->server_port);
        Init_connection_pool(); // 初始化连接池
    }
    else 
    {
        logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->error("Login server registration failed: {} {}", this->server_address, this->server_port);
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
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Login server unregistered successfully: {} {}", this->server_address, this->server_port);
    }
    else
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->error("Login server unregistration failed: {} {}", this->server_address, this->server_port);
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
        logger_manager.getLogger(poor::LogCategory::CONNECTION_POOL)->info("Login server updated connection pools successfully");
    }
    else
    {
        logger_manager.getLogger(poor::LogCategory::CONNECTION_POOL)->error("Failed to get connection pools information");
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
            this->logger_manager.getLogger(poor::LogCategory::HEARTBEAT)->info("Heartbeat sent successfully.");
        }
        else
        {
            this->logger_manager.getLogger(poor::LogCategory::HEARTBEAT)->error("Failed to send heartbeat.");
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

// 登出服务接口
grpc::Status LoginServerImpl::Logout(grpc::ServerContext* context, const rpc_server::LogoutReq* req, rpc_server::LogoutRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_logout(req, res); // 查询的数据库名，表名，查询条件
    });

    // 等待任务完成
    task_future.get();

    return grpc::Status::OK;
}

// 注册服务接口
grpc::Status LoginServerImpl::Register(grpc::ServerContext* context, const rpc_server::RegisterReq* req, rpc_server::RegisterRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_register(req, res); // 查询的数据库名，表名，查询条件
    });

    // 等待任务完成
    task_future.get();
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

// 修改密码服务
grpc::Status LoginServerImpl::Change_password(grpc::ServerContext* context, const rpc_server::ChangePasswordReq* req, rpc_server::ChangePasswordRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_change_password(req, res); // 查询的数据库名，表名，查询条件
    });

    // 等待任务完成
    task_future.get();

    return grpc::Status::OK;
}

// 获取在线用户列表
grpc::Status LoginServerImpl::Is_user_online(grpc::ServerContext* context, const rpc_server::IsUserOnlineReq* req, rpc_server::IsUserOnlineRes* res)
{
    /* 返回在线用户的账号列表
    */
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_is_user_online(req, res); // 查询的数据库名，表名，查询条件
    });

    // 等待任务完成
    task_future.get();

    return grpc::Status::OK;
}

/************************************ gRPC服务接口工具函数 **************************************************/
// 登录服务
void LoginServerImpl::Handle_login(const rpc_server::LoginReq* req, rpc_server::LoginRes* res)
{
    /* 登录服务
    * 通过查询数据库，验证用户的用户名和密码
    * 如果验证成功，生成Token
    * 将账号和Token存储到Redis中，用于验证用户是否在线
    * 更新数据库中的最后登录时间
    * 用户退出时，删除Redis中的账号和Token
    */

    // 获取用户名和密码
    std::string account = req->account();
    std::string password = req->password();

    // 从连接池中获取数据库服务器连接
    auto channel = this->db_connection_pool.get_connection(rpc_server::ServerType::DB);
    auto db_stub = rpc_server::DBServer::NewStub(channel);

    // 构造查询条件
    std::map<std::string, std::string> query = {{"user_account", account}, {"user_password", password}};

    // 构造数据库查询请求
    rpc_server::ReadReq read_request;
    rpc_server::ReadRes read_response;
    grpc::ClientContext client_context;

    read_request.set_database("poor_users");
    read_request.set_table("poor_users");
    for(auto& it : query)
    {
        (*read_request.mutable_query())[it.first] = it.second;
    }

    // 调用数据库服务器的查询服务
    grpc::Status status = db_stub->Read(&client_context, read_request, &read_response);

    if(status.ok() && read_response.success())
    {
        // 生成Token
        std::string token = Make_token(account);

        // 将用户在线状态和token存储到Redis中
        auto client = redis_client.get_client();
        client->set(account, token);   // 保存 key用户账号，value用户token，用于验证用户是否在线
        client->expire(account, 1800); // 设置30分钟过期时间

        res->set_success(true);
        res->set_message("Login successful");
        res->set_account(account);
        res->set_token(token);
        this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Login successful");
    }
    else
    {
        res->set_success(false);
        res->set_message("Login failed");
        this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Login failed");
    }

    this->db_connection_pool.release_connection(rpc_server::ServerType::DB, channel); // 释放数据库服务器连接
}

// 登出服务
void LoginServerImpl::Handle_logout(const rpc_server::LogoutReq* req, rpc_server::LogoutRes* res)    // 登出
{
    /* 登出服务
    * 删除Redis中的账号和Token，表示用户退出
    */

    // 获取账号
    std::string account = req->account();
    std::string token = req->token();

    // 验证Token的有效性
    if(!Validate_token(account, token))
    {
        res->set_success(false);
        res->set_message("token error");
        return;
    }

    // 从Redis中删除用户在线状态和token
    auto client = redis_client.get_client();
    client->del({account});
    client->sync_commit();
    res->set_success(true);
    res->set_message("Logout successful");
    this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Logout successful");
}

// 注册服务
void LoginServerImpl::Handle_register(const rpc_server::RegisterReq* req,rpc_server::RegisterRes* res)    // 注册
{
    /* 注册服务
    * 在数据库中插入用户账号和密码
    * 如果插入成功，生成Token
    * 将账号和Token存储到Redis中，将用户在线状态设置为true
    * 创建文件服务器的文件表（每个用户一个表，注册时创建）
    */

    // 获取用户名和密码
    std::string user_name = req->user_name();
    std::string password = req->password();
    std::string email = req->email();
    std::string phone_number = req->phone();
    std::string id_number = req->id_number();

    // 获取当前时间戳（毫秒）
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    std::ostringstream oss;
    oss << now_ms;

    // 创建用户账号
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> dist(10000000000, 99999999999); // 使用 uuid 生成 11 位数字

    // 检查上传数据（手机号，身份证号可选：默认为111*1）
    if(phone_number == "" || phone_number.size() < 11)
    {
        phone_number = "11111111111";
    }
    if(id_number == "" || id_number.size() < 14)
    {
        id_number = "111111111111111111";
    }

    // 创建用户数据
    std::string account = std::to_string(dist(gen));   // 将用户账号转换为字符串
    std::string avatar = "default.png"; // 头像路径（创建时使用统一的默认头像）
    std::string registration_date = std::to_string(now_ms);    // 注册时间
    std::string last_login_date = std::to_string(now_ms);  // 最后登录时间
    std::string user_status = "active";  // 用户状态

    // 构造插入数据
    std::map<std::string, std::string> data = {
        {"user_account", account},
        {"user_name", user_name},
        {"user_password", password},
        {"email", email},
        {"phone_number", phone_number},
        {"id_number", id_number},
        {"avatar", avatar},
        {"registration_date", registration_date},
        {"last_login_date", last_login_date},
        {"user_status", user_status}
    };

    // 构造数据库插入请求
    rpc_server::CreateReq create_req;
    create_req.set_database("poor_users");  // 插入的数据库
    create_req.set_table("poor_users"); // 插入的表
    for(auto& it : data)
    {
        (*create_req.mutable_data())[it.first] = it.second;
    }

    // 构造响应与客户端上下文
    rpc_server::CreateRes create_res;
    grpc::ClientContext client_context;

    // 从连接池中获取数据库服务器连接
    auto channel = this->db_connection_pool.get_connection(rpc_server::ServerType::DB);
    auto db_stub = rpc_server::DBServer::NewStub(channel);

    // 调用数据库服务器的添加服务
    grpc::Status status = db_stub->Create(&client_context, create_req, &create_res);

    if(status.ok() && create_res.success())
    {
        // 生成Token
        std::string token = Make_token(account);

        // 将用户在线状态和token存储到Redis中
        auto client = redis_client.get_client();
        client->set(account, token);   // 保存 key用户账号，value用户token，用于验证用户是否在线
        client->expire(account, 1800); // 设置30分钟过期时间

        Create_file_table(account); // 为用户创建文件表

        res->set_success(true);
        res->set_message("Register successful");
        res->set_account(account);  // 设置用户账号
        res->set_token(token);  // 设置用户token
        this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Register successful");
    }
    else
    {
        res->set_success(false);
        res->set_message("Register failed");
        this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Register failed");
    }

    this->db_connection_pool.release_connection(rpc_server::ServerType::DB, channel); // 释放数据库服务器连接
}

// 令牌验证服务
void LoginServerImpl::Handle_authenticate(const rpc_server::AuthenticateReq* req, rpc_server::AuthenticateRes* res)
{
    // 获取Token和账号
    std::string token = req->token();
    std::string account = req->account();

    // 验证Token的有效性
    if(Validate_token(account, token))
    {
        res->set_success(true);
        res->set_message("token id ok");
    }
    else
    {
        res->set_success(false);
        res->set_message("Invalid token");
        return;
    }
}

// 修改密码
void LoginServerImpl::Handle_change_password(const rpc_server::ChangePasswordReq* req, rpc_server::ChangePasswordRes* res)
{
    // 获取请求数据
    std::string account = req->account();
    std::string token = req->token();
    std::string old_password = req->old_password();
    std::string new_password = req->new_password();

    // 请求数据验证
    if(!Validate_token(account, token)) // 如果token验证失败
    {
        res->set_success(false);
        res->set_message("token error");
        this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Change password error");
        return;
    }

    // 从连接池中获取数据库服务器连接
    auto channel = this->db_connection_pool.get_connection(rpc_server::ServerType::DB);
    auto db_stub = rpc_server::DBServer::NewStub(channel);

    // 构造数据库更新请求
    rpc_server::UpdateReq update_req;
    rpc_server::UpdateRes update_res;
    grpc::ClientContext client_context;

    update_req.set_database("poor_users");
    update_req.set_table("poor_users");
    // 构造更新条件
    std::map<std::string, std::string> query = {{"user_account", account}, {"user_password", old_password}};
    for(auto& it : query)
    {
        (*update_req.mutable_query())[it.first] = it.second;
    }
    // 构造更新数据
    std::map<std::string, std::string> data = {{"user_password", new_password}};
    for(auto& it : data)
    {
        (*update_req.mutable_data())[it.first] = it.second;
    }

    // 调用数据库服务器的更新服务
    grpc::Status status = db_stub->Update(&client_context, update_req, &update_res);

    if(status.ok() && update_res.success())
    {
        res->set_success(true);
        res->set_message("Change password successful");
        this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Change password successful");
    }
    else
    {
        res->set_success(false);
        res->set_message("new_password error");
        this->logger_manager.getLogger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Change password error");
    }

    this->db_connection_pool.release_connection(rpc_server::ServerType::DB, channel); // 释放数据库服务器连接
}

// 获取在线用户列表
void LoginServerImpl::Handle_is_user_online(const rpc_server::IsUserOnlineReq* req, rpc_server::IsUserOnlineRes* res)
{
    // 获取账号
    std::string account = req->account();

    // 检查用户的在线状态
    auto client = redis_client.get_client();
    auto reply = client->exists({account});
    client->sync_commit();
    if(reply.get().as_integer() == 1)
    {
        res->set_success(true);
        res->set_message("User is online");
        res->set_is_online(true);
    }
    else
    {
        res->set_success(true);
        res->set_message("User is not online");
        res->set_is_online(false);
    }
}

/******************************************** 其他工具函数 ***********************************************/
// 生成 用户token
std::string LoginServerImpl::Make_token(const std::string& account)
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
bool LoginServerImpl::Validate_token(const std::string& account_, const std::string& token_)
{
    try
    {
        auto decoded = jwt::decode(token_);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{"secret"})
            .with_issuer("auth0")
            .with_subject(account_);

        verifier.verify(decoded);

        // 检查用户的在线状态
        auto client = redis_client.get_client();
        auto reply = client->get(account_);
        client->sync_commit();
        if(reply.get().as_string() != token_)    // 没有找到token（用户不在线）
        {
            return false;   
        }

        // 延续Token的有效期
        client->expire(account_, 1800);

        return true;
    }
    catch(const std::exception& e)
    {
        return false;
    }
}

// SHA256哈希加密函数（生成64位16进制数）
std::string LoginServerImpl::SHA256(const std::string& str_) // SHA256哈希加密函数（生成64位16禁止数）
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str_.c_str(), str_.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// 为用户创建文件表
void LoginServerImpl::Create_file_table(const std::string& account)
{
    try
    {
        // 构造表名
        std::string table_name = "file_" + account;

        // 构造字段定义
        std::vector<rpc_server::CreateTableReq::Field> fields;  // 字段容器
        // 用户ID字段
        rpc_server::CreateTableReq::Field user_id_field;    // 字段
        user_id_field.set_name("user_id");  // 字段名
        user_id_field.set_type("BIGINT");   // 字段类型
        user_id_field.set_not_null(true);   // 非空
        user_id_field.set_comment("关联 user_info 表的 id 字段"); // 注释
        fields.push_back(user_id_field);    // 添加字段

        // 原始文件名字段
        rpc_server::CreateTableReq::Field original_name_field;
        original_name_field.set_name("original_file_name");
        original_name_field.set_type("VARCHAR(255)");
        original_name_field.set_not_null(true);
        original_name_field.set_comment("原始文件名");
        fields.push_back(original_name_field);

        // 文件大小字段
        rpc_server::CreateTableReq::Field file_size_field;
        file_size_field.set_name("file_size");
        file_size_field.set_type("BIGINT");
        file_size_field.set_not_null(true);
        file_size_field.set_comment("文件大小");
        fields.push_back(file_size_field);

        // 文件哈希值字段
        rpc_server::CreateTableReq::Field file_hash_field;
        file_hash_field.set_name("file_hash");
        file_hash_field.set_type("VARCHAR(64)");
        file_hash_field.set_not_null(true);
        file_hash_field.set_comment("文件哈希值");
        fields.push_back(file_hash_field);

        // 服务器保存文件名字段
        rpc_server::CreateTableReq::Field server_file_name_field;
        server_file_name_field.set_name("server_file_name");
        server_file_name_field.set_type("VARCHAR(255)");
        server_file_name_field.set_not_null(true);
        server_file_name_field.set_comment("服务器保存的文件名");
        fields.push_back(server_file_name_field);

        // 文件上传时间字段（时间戳）
        rpc_server::CreateTableReq::Field upload_time_field;
        upload_time_field.set_name("upload_time");
        upload_time_field.set_type("VARCHAR(20)");
        upload_time_field.set_not_null(true);
        upload_time_field.set_comment("文件上传时间（毫秒时间戳）");
        fields.push_back(upload_time_field);

        // 文件状态字段（枚举类型：上传中，缺损，正常）
        rpc_server::CreateTableReq::Field status_field;
        status_field.set_name("status");
        status_field.set_type("ENUM('upload_ing', 'coloboma', 'normal')"); // 定义枚举类型
        status_field.set_not_null(true);
        status_field.set_comment("文件状态：上传中，缺损，正常");
        fields.push_back(status_field);

        // 构造主键约束
        rpc_server::CreateTableReq::Constraint primary_key_constraint;
        primary_key_constraint.set_type("PRIMARY_KEY");
        primary_key_constraint.add_fields("user_id");

        // 构造建表请求
        rpc_server::CreateTableReq create_table_req;
        create_table_req.set_database("poor_file_hub");
        create_table_req.set_table(table_name);
        for(const auto& field : fields)
        {
            *create_table_req.add_fields() = field;
        }
        *create_table_req.add_constraints() = primary_key_constraint;
        create_table_req.set_engine("InnoDB");
        create_table_req.set_charset("utf8mb4");
        create_table_req.set_collation("utf8mb4_general_ci");
        create_table_req.set_table_comment("用户文件表");

        // 构造响应与客户端上下文
        rpc_server::CreateTableRes create_table_res;
        grpc::ClientContext client_context;

        // 从连接池中获取数据库服务器连接
        auto channel = db_connection_pool.get_connection(rpc_server::ServerType::DB);
        auto db_stub = rpc_server::DBServer::NewStub(channel);

        // 调用数据库服务器的建表服务
        grpc::Status status = db_stub->Create_table(&client_context, create_table_req, &create_table_res);

        if(status.ok() && create_table_res.success())
        {
            this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->info("Table {} created successfully in poor_file_hub", table_name);
        }
        else
        {
            logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("Failed to create table {}: {}", table_name, create_table_res.message());
        }

        // 释放数据库服务器连接
        db_connection_pool.release_connection(rpc_server::ServerType::DB, channel);
    }
    catch(const std::exception& e)
    {
        logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("Exception in Create_file_table: {}", e.what());
    }
}
