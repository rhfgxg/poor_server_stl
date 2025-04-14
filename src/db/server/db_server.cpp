#include "db_server.h"

DBServerImpl::DBServerImpl(LoggerManager& logger_manager_, const std::string address, const std::string port):
    server_type(rpc_server::ServerType::DB),
    server_address(address),
    server_port(port),
    logger_manager(logger_manager_),
    central_stub(rpc_server::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))) // 中心服务器存根
{
    lua_State* L = luaL_newstate();  // 创建lua虚拟机
    luaL_openlibs(L);   // 打开lua标准库

    // 读取配置文件并初始化所有数据库连接池
    std::string db_uri = Read_db_config(L, "config/cfg_db.lua", "poor_users");
    poor_users_pool = std::make_unique<DBConnectionPool>(db_uri, 10);   // 用户数据库连接池

    db_uri = Read_db_config(L, "config/cfg_db.lua", "poor_file_hub");
    poor_file_pool = std::make_unique<DBConnectionPool>(db_uri, 10);   // 网盘数据库连接池

    db_uri = Read_db_config(L, "config/cfg_db.lua", "hearthstone");
    hearthstone_pool = std::make_unique<DBConnectionPool>(db_uri, 10);   // 炉石数据库连接池

    lua_close(L);   // 关闭lua虚拟机

    register_server(); // 注册服务器

    // 启动定时任务
    // 定时向中心服务器发送心跳包
    std::thread(&DBServerImpl::Send_heartbeat, this).detach();
}

DBServerImpl::~DBServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    unregister_server(); // 注销服务器

    // 记录关闭日志
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("DBServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

/*************************************** 多线程工具函数 *****************************************************************/
// 启动线程池
void DBServerImpl::start_thread_pool(int num_threads)
{// 相关注释请参考 /src/central/src/central/central_server.cpp/start_thread_pool()
    for(int i = 0; i < num_threads; ++i)
    {
        this->thread_pool.emplace_back(&DBServerImpl::Worker_thread, this);
    }
}

// 停止线程池
void DBServerImpl::stop_thread_pool()
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
    // 清空任务队列
    std::queue<std::function<void()>> empty;
    std::swap(this->task_queue,empty);
}

// 添加异步任务
std::future<void> DBServerImpl::add_async_task(std::function<void()> task)
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
void DBServerImpl::Worker_thread()
{// 相关注释请参考 /src/central/src/central/central_server.cpp/worker_thread()
    while(true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);

            this->queue_cv.wait(lock,[this] {   // 等待任务队列不为空或线程池停止
                return !this->task_queue.empty() || this->stop_threads;
            });

            if(this->stop_threads && this->task_queue.empty())  // 线程池停止
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
// 注册服务器
void DBServerImpl::register_server()
{
    // 创建请求
    rpc_server::RegisterServerReq request;
    request.set_server_type(this->server_type);
    request.set_address(this->server_address);
    request.set_port(this->server_port);
    // 创建响应
    rpc_server::RegisterServerRes response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->Register_server(&context, request, &response);

    if(status.ok() && response.success())
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("DB server registered successfully: {} {}",this->server_address,this->server_port);
    }
    else
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->error("DB server registration failed: {} {}",this->server_address,this->server_port);
    }
}

// 注销服务器
void DBServerImpl::unregister_server()
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

    grpc::Status status = central_stub->Unregister_server(&context, request, &response);

    if(status.ok() && response.success())
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Server unregistration successful: {}", response.message());
    }
    else
    {
        this->logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->error("Server unregistration failed:  {}",response.message());
    }
}

/*************************************** 定时任务 *********************************************************/
// 定时任务：发送心跳包
void DBServerImpl::Send_heartbeat()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒发送一次心跳包

        rpc_server::HeartbeatReq request;
        rpc_server::HeartbeatRes response;
        grpc::ClientContext context;

        request.set_server_type(rpc_server::ServerType::DB);
        request.set_address(this->server_address); // 设置服务器ip
        request.set_port(this->server_port); // 设置服务器端口

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

/**************************************** grpc服务接口定义 **************************************************************************/
// 添加数据
grpc::Status DBServerImpl::Create(grpc::ServerContext* context, const rpc_server::CreateReq* req, rpc_server::CreateRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_create(req, res);
    });

    // 等待任务完成
    task_future.get();
    return grpc::Status::OK;
}

// 查询数据
grpc::Status DBServerImpl::Read(grpc::ServerContext* context, const rpc_server::ReadReq* req, rpc_server::ReadRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {  
        this->Handle_read(req, res);
    }); // 添加异步任务：查询数据库

    // 等待任务完成
    task_future.get();
    return grpc::Status::OK;
}

// 更新数据
grpc::Status DBServerImpl::Update(grpc::ServerContext* context, const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res)
{
    auto task_future = this->add_async_task([this,req,res] {
        this->Handle_update(req, res);
    });

    // 等待任务完成
    task_future.get();
    return grpc::Status::OK;
}

// 删除数据
grpc::Status DBServerImpl::Delete(grpc::ServerContext* context, const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_delete(req, res);
    });

    // 等待任务完成
    task_future.get();
    return grpc::Status::OK;
}

// 新建表
grpc::Status DBServerImpl::Create_table(grpc::ServerContext* context, const rpc_server::CreateTableReq* req, rpc_server::CreateTableRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_create_table(req, res);
    });

    // 等待任务完成
    task_future.get();
    return grpc::Status::OK;
}

/**************************************** grpc接口工具函数 **************************************************************************/
// 处理数据库操作的函数
// 添加
void DBServerImpl::Handle_create(const rpc_server::CreateReq* req, rpc_server::CreateRes* res)
{
    try
    {
        // 获取请求参数
        std::string db_name = req->database();
        std::string tab_name = req->table();
        const auto& data = req->data();

        std::unique_ptr<mysqlx::Session> session = this->Git_connection(db_name);   // 通过数据库名获取数据库连接
        mysqlx::Table table = session->getSchema(db_name).getTable(tab_name);   // 选择表

        // 构建插入语句
        std::vector<std::string> columns;
        std::vector<mysqlx::Value> values;
        for(const auto& field : data)
        {
            columns.push_back(field.first);
            values.push_back(field.second);
        }

        // 执行插入语句
        table.insert(columns).values(values).execute();

        // 设置响应
        res->set_success(true);
        res->set_message("Create successful");
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->info("DB create successful");

        // 释放数据库连接
        this->Release_connection(db_name, std::move(session));
    }
    catch(const std::exception& e)
    {
        // 设置响应
        res->set_success(false);
        res->set_message("Create failed: " + std::string(e.what()));
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("DB create failed: {}", e.what());
    }
}

// 查询
void DBServerImpl::Handle_read(const rpc_server::ReadReq* req, rpc_server::ReadRes* res)
{
    try
    {
        // 获取请求的 query 参数
        const std::string& db_name = req->database();
        const std::string& tab_name = req->table();
        const auto& query = req->query();

        std::unique_ptr<mysqlx::Session> session = this->Git_connection(db_name);   // 通过数据库名获取数据库连接
        mysqlx::Table table = session->getSchema(db_name).getTable(tab_name);   // 选择表

        // 构建查询条件
        std::string condition;
        for(const auto& q : query)
        {
            if(!condition.empty())
            {
                condition += " AND ";
            }
            condition += q.first + " = '" + q.second + "'";
        }

        // 执行查询
        mysqlx::RowResult result = table.select("*").where(condition).execute();

        // 设置响应：查询结果
        for(mysqlx::Row row : result)
        {
            rpc_server::Result* response_result = res->add_results();
            for(size_t i = 0; i < row.colCount(); ++i)
            {
                std::string column_name = result.getColumn(i).getColumnName();
                std::string column_value;
                if(row[i].isNull())
                {
                    column_value = "NULL";
                } else if(row[i].getType() == mysqlx::Value::Type::STRING)
                {
                    column_value = row[i].get<std::string>();
                } else if(row[i].getType() == mysqlx::Value::Type::VNULL)
                {
                    column_value = "VNULL";
                } else if(row[i].getType() == mysqlx::Value::Type::INT64)
                {
                    column_value = std::to_string(row[i].get<int64_t>());
                } else if(row[i].getType() == mysqlx::Value::Type::UINT64)
                {
                    column_value = std::to_string(row[i].get<uint64_t>());
                } else if(row[i].getType() == mysqlx::Value::Type::FLOAT)
                {
                    column_value = std::to_string(row[i].get<float>());
                } else if(row[i].getType() == mysqlx::Value::Type::DOUBLE)
                {
                    column_value = std::to_string(row[i].get<double>());
                } else if(row[i].getType() == mysqlx::Value::Type::BOOL)
                {
                    column_value = row[i].get<bool>() ? "true" : "false";
                } else if(row[i].getType() == mysqlx::Value::Type::RAW)
                {
                    column_value = "Raw Data"; // 需要进一步处理原始数据
                } else
                {
                    column_value = "Unsupported Type";
                }
                response_result->mutable_fields()->insert({column_name, column_value});
            }
        }

        res->set_success(true);
        res->set_message("Query successful");
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->info("DB read successful");

        // 释放数据库连接
        this->Release_connection(db_name, std::move(session));
    }
    catch(const std::exception& e)
    {
        res->set_success(false);
        res->set_message("Query failed: " + std::string(e.what()));
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("DB read failed: {}", e.what());
    }
}

// 更新
void DBServerImpl::Handle_update(const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res)
{
    try
    {
        // 获取请求参数
        std::string db_name = req->database();
        std::string tab_name = req->table();
        const auto& query = req->query();
        const auto& data = req->data();

        std::unique_ptr<mysqlx::Session> session = this->Git_connection(db_name);   // 通过数据库名获取数据库连接
        mysqlx::Table table = session->getSchema(db_name).getTable(tab_name);   // 选择表

        // 构建查询条件
        std::string condition;
        for(const auto& q : query)
        {
            if(!condition.empty())
            {
                condition += " AND ";
            }
            condition += q.first + " = '" + q.second + "'";
        }
        // 构建更新数据
        mysqlx::TableUpdate update = table.update();
        for(const auto& d : data)
        {
            update.set(d.first, mysqlx::Value(d.second));
        }
        // 执行更新
        update.where(condition).execute();
        // 设置响应
        res->set_success(true);
        res->set_message("Update successful");
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->info("DB update successful");

        // 释放数据库连接
        this->Release_connection(db_name, std::move(session));
    }
    catch(const std::exception& e)
    {
        // 设置响应
        res->set_success(false);
        res->set_message("Update failed: " + std::string(e.what()));
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("DB update failed: {}", e.what());
    }
}

// 删除
void DBServerImpl::Handle_delete(const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res)
{
    // 获取请求参数
    std::string db_name = req->database();
    std::string tab_name = req->table();

    std::unique_ptr<mysqlx::Session> session = this->Git_connection(db_name);   // 通过数据库名获取数据库连接
    mysqlx::Table table = session->getSchema(db_name).getTable(tab_name);   // 选择表

    // 假设数据库操作
    // ...
    // 设置响应参数
    res->set_success(true);
    res->set_message("Delete successful");
    std::cout << "Database delete successful" << std::endl;

    // 释放数据库连接
    this->Release_connection(db_name, std::move(session));
}

// 新建表
void DBServerImpl::Handle_create_table(const rpc_server::CreateTableReq* req, rpc_server::CreateTableRes* res)
{
    try
    {
        // 获取请求参数
        std::string db_name = req->database();
        std::string tab_name = req->table();

        std::unique_ptr<mysqlx::Session> session = this->Git_connection(db_name);   // 通过数据库名获取数据库连接

        // 拼接建表语句
        std::string sql = "CREATE TABLE `" + tab_name + "` (";
        // 添加字段定义
        for(const auto& field : req->fields())
        {
            sql += "`" + field.name() + "` " + field.type();
            if(field.not_null())
            {
                sql += " NOT NULL";
            }
            if(!field.default_value().empty())
            {
                sql += " DEFAULT " + field.default_value();
            }
            if(field.auto_increment())
            {
                sql += " AUTO_INCREMENT";
            }
            if(!field.comment().empty())
            {
                sql += " COMMENT '" + field.comment() + "'";
            }
            sql += ",";
        }
        // 添加表级约束
        for(const auto& constraint : req->constraints())
        {
            if(constraint.type() == "PRIMARY_KEY")
            {
                sql += "PRIMARY KEY (";
                for(const auto& field : constraint.fields())
                {
                    sql += "`" + field + "`,";
                }
                sql.pop_back(); // 移除最后一个逗号
                sql += "),";
            }
            else if(constraint.type() == "UNIQUE")
            {
                sql += "UNIQUE KEY `" + constraint.name() + "` (";
                for(const auto& field : constraint.fields())
                {
                    sql += "`" + field + "`,";
                }
                sql.pop_back();
                sql += "),";
            }
        }
        sql.pop_back(); // 移除最后一个逗号
        sql += ")";
        // 添加表选项
        if(!req->engine().empty())
        {
            sql += " ENGINE = " + req->engine();
        }
        if(!req->charset().empty())
        {
            sql += " CHARACTER SET = " + req->charset();
        }
        if(!req->collation().empty())
        {
            sql += " COLLATE = " + req->collation();
        }
        if(!req->table_comment().empty())
        {
            sql += " COMMENT = '" + req->table_comment() + "'";
        }
        sql += " ROW_FORMAT = Dynamic;";

        // 执行建表语句
        session->sql(sql).execute();

        res->set_success(true);
        res->set_message("Table created successfully");
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->info("DB make table successful");

        this->Release_connection(db_name, std::move(session));
    }
    catch(const std::exception& e)
    {
        // 设置响应
        res->set_success(false);
        res->set_message("Create table failed: " + std::string(e.what()));
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("DB make table failed: {}", e.what());
    }
}

/******************************************** 其他工具函数 *****************************************************/
// 读取 数据库配置配置文件，获得指定数据库的连接字符串
std::string DBServerImpl::Read_db_config(lua_State* L_, const std::string& file_url_, const std::string& db_name_)
{
    // 加载 Lua 配置文件
    if(luaL_dofile(L_, file_url_.c_str()) != LUA_OK)
    {
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("Failed to load Lua file: {}", lua_tostring(L_, -1));
        lua_pop(L_, 1);
        return "";
    }

    // 获取返回值（配置表）
    if(!lua_istable(L_, -1))
    {
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("db_config is not a table in {}", file_url_);
        lua_pop(L_, 1);
        return "";
    }

    // 获取 "mysqlx" 表
    lua_getfield(L_, -1, "mysqlx");
    if(!lua_istable(L_, -1))
    {
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("mysqlx is not defined in {}", file_url_);
        lua_pop(L_, 1); // Pop db_config
        return "";
    }


    // 获取指定数据库配置
    lua_getfield(L_, -1, db_name_.c_str());
    if(!lua_istable(L_, -1))
    {
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("Database {} is not defined in {}", db_name_, file_url_);
        lua_pop(L_, 2); // Pop db_name_ and db_config
        return "";
    }

    // 提取字段
    auto get_field = [&](const char* field_name) -> std::string
    {
        lua_getfield(L_, -1, field_name);
        if(!lua_isstring(L_, -1))
        {
            this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->warn("Field {} is missing or invalid for database {}", field_name, db_name_);
            lua_pop(L_, 1);
            return "";
        }
        std::string value = lua_tostring(L_, -1);
        lua_pop(L_, 1);
        return value;
    };

    std::string host = get_field("host");
    std::string port = get_field("port");
    std::string username = get_field("user_name");
    std::string password = get_field("password");

    // 检查必需字段是否存在
    if(host.empty() || port.empty() || username.empty() || password.empty())
    {
        this->logger_manager.getLogger(poor::LogCategory::DATABASE_OPERATIONS)->error("Incomplete configuration for database {}", db_name_);
        lua_pop(L_, 2); // Pop db_name_ and db_config
        return "";
    }

    // 构造连接字符串
    std::string db_uri = "mysqlx://" + username + ":" + password + "@" + host + ":" + port + "/" + db_name_;

    lua_pop(L_, 2); // Pop db_name_ and db_config
    return db_uri;
}

// 获取数据库连接
std::unique_ptr<mysqlx::Session> DBServerImpl::Git_connection(const std::string& db_name)
{
    if(db_name == "poor_users")
    {
        return this->poor_users_pool->Get_connection();
    }
    else if(db_name == "poor_file_hub")
    {
        return this->poor_file_pool->Get_connection();
    }
    else if(db_name == "hearthstone")
    {
        return this->hearthstone_pool->Get_connection();
    }
    else
    {
        throw std::runtime_error("Unknown database name: " + db_name);
    }
}

// 释放数据库连接
void DBServerImpl::Release_connection(const std::string& db_name, std::unique_ptr<mysqlx::Session> session)
{
    if(db_name == "poor_users")
    {
        this->poor_users_pool->Release_connection(std::move(session));
    }
    else if(db_name == "poor_file_hub")
    {
        this->poor_file_pool->Release_connection(std::move(session));
    }
    else if(db_name == "hearthstone")
    {
        this->hearthstone_pool->Release_connection(std::move(session));
    }
    else
    {
        throw std::runtime_error("Unknown database name: " + db_name);
    }
}
