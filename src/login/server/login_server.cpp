#include "login_server.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>

// 构造函数 - 继承 BaseServer
LoginServerImpl::LoginServerImpl(LoggerManager& logger_manager_, const std::string address, std::string port)
    : BaseServer(rpc_server::ServerType::LOGIN, address, port, logger_manager_, 4),  // 4 个线程
      db_connection_pool(10)
{
    // Redis 连接将在 on_start() 中初始化
}

// 析构函数
LoginServerImpl::~LoginServerImpl()
{
    // 停止连接池更新线程
    pool_update_running_.store(false);
    if (update_pool_thread_.joinable())
    {
        update_pool_thread_.join();
    }
    
    log_shutdown("LoginServer stopped");
}

// ==================== BaseServer 钩子方法 ====================

bool LoginServerImpl::on_start()
{
    log_startup("LoginServer initializing...");
    
    // 连接 Redis
    if (!redis_client.connect_from_config())
    {
        log_startup("Failed to connect to Redis");
        return false;
    }
    log_startup("Redis connection successful");
    
    log_startup("LoginServer initialized successfully");
    return true;
}

void LoginServerImpl::on_stop()
{
    log_shutdown("LoginServer shutting down...");
    
    // 停止连接池更新线程
    pool_update_running_.store(false);
    if (update_pool_thread_.joinable())
    {
        update_pool_thread_.join();
    }
    
    log_shutdown("LoginServer shutdown complete");
}

void LoginServerImpl::on_registered(bool success)
{
    if (success)
    {
        // 注册成功后，初始化连接池
        Init_connection_pool();
        
        // 启动定时更新连接池的线程
        pool_update_running_.store(true);
        update_pool_thread_ = std::thread(&LoginServerImpl::Update_connection_pool, this);
        
        log_startup("Login connection pools initialized and update thread started");
    }
    else
    {
        log_startup("Failed to register, skipping connection pool initialization");
    }
}

// ==================== 连接池管理 ====================

void LoginServerImpl::Init_connection_pool()
{
    try
    {
        auto channel = central_connection_pool_->get_connection(rpc_server::ServerType::CENTRAL);
        auto stub = rpc_server::CentralServer::NewStub(channel);
        
        rpc_server::MultipleConnectPoorReq req;
        req.add_server_types(rpc_server::ServerType::DB);
        
        rpc_server::MultipleConnectPoorRes res;
        grpc::ClientContext context;
        
        grpc::Status status = stub->Get_connec_poor(&context, req, &res);
        
        central_connection_pool_->release_connection(rpc_server::ServerType::CENTRAL, channel);
        
        if (status.ok() && res.success())
        {
            for (const rpc_server::ConnectPool& connect_pool : res.connect_pools())
            {
                for (const rpc_server::ConnectInfo& conn_info : connect_pool.connect_info())
                {
                    if (connect_pool.server_type() == rpc_server::ServerType::DB)
                    {
                        db_connection_pool.add_server(
                            rpc_server::ServerType::DB,
                            conn_info.address(),
                            std::to_string(conn_info.port())
                        );
                    }
                }
            }
            
            get_logger(poor::LogCategory::CONNECTION_POOL)->info("Login server updated connection pools successfully");
        }
        else
        {
            get_logger(poor::LogCategory::CONNECTION_POOL)->error("Failed to get connection pools information");
        }
    }
    catch (const std::exception& e)
    {
        get_logger(poor::LogCategory::CONNECTION_POOL)->error("Exception during connection pool initialization: {}", e.what());
    }
}

void LoginServerImpl::Update_connection_pool()
{
    while (pool_update_running_.load())
    {
        std::this_thread::sleep_for(std::chrono::minutes(5));
        
        if (pool_update_running_.load())
        {
            Init_connection_pool();
        }
    }
}

// ==================== gRPC 服务接口 ====================

grpc::Status LoginServerImpl::Login(grpc::ServerContext* context [[maybe_unused]], const rpc_server::LoginReq* req, rpc_server::LoginRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_login(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status LoginServerImpl::Logout(grpc::ServerContext* context [[maybe_unused]], const rpc_server::LogoutReq* req, rpc_server::LogoutRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_logout(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status LoginServerImpl::Register(grpc::ServerContext* context [[maybe_unused]], const rpc_server::RegisterReq* req, rpc_server::RegisterRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_register(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status LoginServerImpl::Authenticate(grpc::ServerContext* context [[maybe_unused]], const rpc_server::AuthenticateReq* req, rpc_server::AuthenticateRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_authenticate(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status LoginServerImpl::Change_password(grpc::ServerContext* context [[maybe_unused]], const rpc_server::ChangePasswordReq* req, rpc_server::ChangePasswordRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_change_password(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status LoginServerImpl::Is_user_online(grpc::ServerContext* context [[maybe_unused]], const rpc_server::IsUserOnlineReq* req, rpc_server::IsUserOnlineRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_is_user_online(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

// ==================== 业务处理函数 ====================

void LoginServerImpl::Handle_login(const rpc_server::LoginReq* req, rpc_server::LoginRes* res)
{
    std::string account = req->account();
    std::string password = req->password();

    // 从连接池中获取数据库服务器连接
    auto channel = db_connection_pool.get_connection(rpc_server::ServerType::DB);
    if (!channel)
    {
        res->set_success(false);
        res->set_message("Database service unavailable");
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error("Login failed: no DB connection available for {}", account);
        return;
    }
    
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

        if (!redis_client.set_with_expire(account, token, std::chrono::seconds(1800)))
        {
            get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn("Failed to cache token in Redis for {}", account);
        }

        res->set_success(true);
        res->set_message("Login successful");
        res->set_account(account);
        res->set_token(token);
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Login successful: {}", account);
    }
    else
    {
        res->set_success(false);
        res->set_message("Login failed");
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Login failed: {}", account);
    }

    db_connection_pool.release_connection(rpc_server::ServerType::DB, channel); // 释放数据库服务器连接
}

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
    if (!redis_client.delete_key(account))
    {
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn("Failed to delete Redis token for {}", account);
    }
    res->set_success(true);
    res->set_message("Logout successful");
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Logout successful: {}", account);
}

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
    auto channel = db_connection_pool.get_connection(rpc_server::ServerType::DB);
    if (!channel)
    {
        res->set_success(false);
        res->set_message("Database service unavailable");
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error("Register failed: no DB connection available");
        return;
    }
    auto db_stub = rpc_server::DBServer::NewStub(channel);

    // 调用数据库服务器的添加服务
    grpc::Status status = db_stub->Create(&client_context, create_req, &create_res);

    if(status.ok() && create_res.success())
    {
        // 生成Token
        std::string token = Make_token(account);

        if (!redis_client.set_with_expire(account, token, std::chrono::seconds(1800)))
        {
            get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn("Failed to cache token in Redis for {}", account);
        }

        Create_file_table(account); // 为用户创建文件表

        res->set_success(true);
        res->set_message("Register successful");
        res->set_account(account);  // 设置用户账号
        res->set_token(token);  // 设置用户token
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Register successful: {}", account);
    }
    else
    {
        res->set_success(false);
        res->set_message("Register failed");
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Register failed");
    }

    db_connection_pool.release_connection(rpc_server::ServerType::DB, channel); // 释放数据库服务器连接
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
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Change password error: invalid token");
        return;
    }

    // 从连接池中获取数据库服务器连接
    auto channel = db_connection_pool.get_connection(rpc_server::ServerType::DB);
    if (!channel)
    {
        res->set_success(false);
        res->set_message("Database service unavailable");
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error("Change password failed: no DB connection available for {}", account);
        return;
    }
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
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Change password successful: {}", account);
    }
    else
    {
        res->set_success(false);
        res->set_message("new_password error");
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Change password error");
    }

    db_connection_pool.release_connection(rpc_server::ServerType::DB, channel); // 释放数据库服务器连接
}

void LoginServerImpl::Handle_is_user_online(const rpc_server::IsUserOnlineReq* req, rpc_server::IsUserOnlineRes* res)
{
    // 获取账号
    std::string account = req->account();

    bool online = redis_client.key_exists(account);
    res->set_success(true);
    res->set_is_online(online);
    res->set_message(online ? "User is online" : "User is not online");
}

// ==================== 工具函数 ====================

std::string LoginServerImpl::Make_token(const std::string& account)
{
    auto token = jwt::create()
        .set_issuer("auth0")
        .set_type("JWS")
        .set_subject(account)
        .set_audience("example.com")
        .set_issued_at(std::chrono::system_clock::now())
        .sign(jwt::algorithm::hs256{"secret"});

    return token;
}

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

        auto stored_token = redis_client.get_string(account_);
        if (!stored_token || *stored_token != token_)
        {
            return false;
        }

        redis_client.refresh_ttl(account_, std::chrono::seconds(1800));
        return true;
    }
    catch(const std::exception& e)
    {
        (void)e;
        return false;
    }
}

std::string LoginServerImpl::SHA256(const std::string& str_)
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

void LoginServerImpl::Create_file_table(const std::string& account)
{
    try
    {
        std::string table_name = "file_" + account;

        std::vector<rpc_server::CreateTableReq::Field> fields;
        
        rpc_server::CreateTableReq::Field user_id_field;
        user_id_field.set_name("user_id");
        user_id_field.set_type("BIGINT");
        user_id_field.set_not_null(true);
        user_id_field.set_comment("关联 user_info 表的 id 字段");
        fields.push_back(user_id_field);

        rpc_server::CreateTableReq::Field original_name_field;
        original_name_field.set_name("original_file_name");
        original_name_field.set_type("VARCHAR(255)");
        original_name_field.set_not_null(true);
        original_name_field.set_comment("原始文件名");
        fields.push_back(original_name_field);

        rpc_server::CreateTableReq::Field file_size_field;
        file_size_field.set_name("file_size");
        file_size_field.set_type("BIGINT");
        file_size_field.set_not_null(true);
        file_size_field.set_comment("文件大小");
        fields.push_back(file_size_field);

        rpc_server::CreateTableReq::Field file_hash_field;
        file_hash_field.set_name("file_hash");
        file_hash_field.set_type("VARCHAR(64)");
        file_hash_field.set_not_null(true);
        file_hash_field.set_comment("文件哈希值");
        fields.push_back(file_hash_field);

        rpc_server::CreateTableReq::Field server_file_name_field;
        server_file_name_field.set_name("server_file_name");
        server_file_name_field.set_type("VARCHAR(255)");
        server_file_name_field.set_not_null(true);
        server_file_name_field.set_comment("服务器保存的文件名");
        fields.push_back(server_file_name_field);

        rpc_server::CreateTableReq::Field upload_time_field;
        upload_time_field.set_name("upload_time");
        upload_time_field.set_type("VARCHAR(20)");
        upload_time_field.set_not_null(true);
        upload_time_field.set_comment("文件上传时间（毫秒时间戳）");
        fields.push_back(upload_time_field);

        rpc_server::CreateTableReq::Field status_field;
        status_field.set_name("status");
        status_field.set_type("ENUM('upload_ing', 'coloboma', 'normal')");
        status_field.set_not_null(true);
        status_field.set_comment("文件状态：上传中，缺损，正常");
        fields.push_back(status_field);

        rpc_server::CreateTableReq::Constraint primary_key_constraint;
        primary_key_constraint.set_type("PRIMARY_KEY");
        primary_key_constraint.add_fields("user_id");

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

        rpc_server::CreateTableRes create_table_res;
        grpc::ClientContext client_context;

        auto channel = db_connection_pool.get_connection(rpc_server::ServerType::DB);
        if (!channel)
        {
            get_logger(poor::LogCategory::DATABASE_OPERATIONS)->error("Failed to create table {}: no DB connection available", table_name);
            return;
        }
        auto db_stub = rpc_server::DBServer::NewStub(channel);

        grpc::Status status = db_stub->Create_table(&client_context, create_table_req, &create_table_res);

        if(status.ok() && create_table_res.success())
        {
            get_logger(poor::LogCategory::DATABASE_OPERATIONS)->info("Table {} created successfully in poor_file_hub", table_name);
        }
        else
        {
            get_logger(poor::LogCategory::DATABASE_OPERATIONS)->error("Failed to create table {}: {}", table_name, create_table_res.message());
        }

        db_connection_pool.release_connection(rpc_server::ServerType::DB, channel);
    }
    catch(const std::exception& e)
    {
        get_logger(poor::LogCategory::DATABASE_OPERATIONS)->error("Exception in Create_file_table: {}", e.what());
    }
}
