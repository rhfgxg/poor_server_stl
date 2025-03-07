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

    // 读取配置文件并初始化数据库连接池
    std::string db_uri = Read_db_config(L, "config/cfg_db.lua");
    user_db_pool = std::make_unique<DBConnectionPool>(db_uri, 10);   // 初始化数据库连接池

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
    logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->info("DBServer stopped");
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
        this->logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->info("DB server registered successfully: {} {}",this->server_address,this->server_port);
    }
    else
    {
        this->logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->error("DB server registration failed: {} {}",this->server_address,this->server_port);
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
        this->logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->info("Server unregistration successful: {}", response.message());
    }
    else
    {
        this->logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->error("Server unregistration failed:  {}",response.message());
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
            this->logger_manager.getLogger(rpc_server::LogCategory::HEARTBEAT)->info("Heartbeat sent successfully.");
        }
         else
        {
            this->logger_manager.getLogger(rpc_server::LogCategory::HEARTBEAT)->error("Failed to send heartbeat.");
        }
    }
}

/**************************************** grpc服务接口定义 **************************************************************************/
// 添加
grpc::Status DBServerImpl::Create(grpc::ServerContext* context, const rpc_server::CreateReq* req, rpc_server::CreateRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_create(req, res);
    });

    // 等待任务完成
    task_future.get();
    return grpc::Status::OK;
}

// 查询
grpc::Status DBServerImpl::Read(grpc::ServerContext* context, const rpc_server::ReadReq* req, rpc_server::ReadRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {  
        this->Handle_read(req, res);
    }); // 添加异步任务：查询数据库

    // 等待任务完成
    task_future.get();
    return grpc::Status::OK;
}

// 更新
grpc::Status DBServerImpl::Update(grpc::ServerContext* context, const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res)
{
    auto task_future = this->add_async_task([this,req,res] {
        this->Handle_update(req, res);
    });

    // 等待任务完成
    task_future.get();
    return grpc::Status::OK;
}

// 删除
grpc::Status DBServerImpl::Delete(grpc::ServerContext* context, const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_delete(req, res);
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
    // 获取数据库连接
    mysqlx::Session session = this->user_db_pool->get_connection();

    // 获取请求参数
    std::string db_name = req->database();
    std::string tab_name = req->table();
    // 假设数据库操作
    // ...
    // 设置响应参数
    res->set_success(true);
    res->set_message("Create successful");
    std::cout << "DB created successfully" << std::endl;

    // 释放数据库连接
    this->user_db_pool->release_connection(std::move(session));
}

// 查询
void DBServerImpl::Handle_read(const rpc_server::ReadReq* req, rpc_server::ReadRes* res)
{
    // 获取数据库连接
    mysqlx::Session session = user_db_pool->get_connection();

    // 获取请求的 query 参数
    const std::string& database = req->database();
    const std::string& table = req->table();
    const auto& query = req->query();

    // 选择数据库
    mysqlx::Table tbl = session.getSchema(database).getTable(table);

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
    mysqlx::RowResult result = tbl.select("*").where(condition).execute();

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
            }
            else if(row[i].getType() == mysqlx::Value::Type::STRING)
            {
                column_value = row[i].get<std::string>();
            }
            else if(row[i].getType() == mysqlx::Value::Type::VNULL)
            {
                column_value = "VNULL";
            }
            else if(row[i].getType() == mysqlx::Value::Type::INT64)
            {
                column_value = std::to_string(row[i].get<int64_t>());
            }
            else if(row[i].getType() == mysqlx::Value::Type::UINT64)
            {
                column_value = std::to_string(row[i].get<uint64_t>());
            }
            else if(row[i].getType() == mysqlx::Value::Type::FLOAT)
            {
                column_value = std::to_string(row[i].get<float>());
            }
            else if(row[i].getType() == mysqlx::Value::Type::DOUBLE)
            {
                column_value = std::to_string(row[i].get<double>());
            }
            else if(row[i].getType() == mysqlx::Value::Type::BOOL)
            {
                column_value = row[i].get<bool>() ? "true" : "false";
            }
            else if(row[i].getType() == mysqlx::Value::Type::RAW)
            {
                column_value = "Raw Data"; // 需要进一步处理原始数据
            }
            else
            {
                column_value = "Unsupported Type";
            }
            response_result->mutable_fields()->insert({column_name, column_value});
        }
    }

    this->logger_manager.getLogger(rpc_server::LogCategory::DATABASE_OPERATIONS)->info("Database query operation successful: Database: {}", database);

    res->set_success(true);
    res->set_message("Query successful");

    // 释放数据库连接
    this->user_db_pool->release_connection(std::move(session));
}

// 更新
void DBServerImpl::Handle_update(const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res)
{
    // 获取数据库连接
    mysqlx::Session session = this->user_db_pool->get_connection();

    // 获取请求参数
    std::string db_name = req->database();
    std::string tab_name = req->table();
    // 假设数据库操作
    // ...
    // 设置响应参数
    res->set_success(true);
    res->set_message("Update successful");
    std::cout << "Database update successful" << std::endl;

    // 释放数据库连接
    this->user_db_pool->release_connection(std::move(session));
}

// 删除
void DBServerImpl::Handle_delete(const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res)
{
    // 获取数据库连接
    mysqlx::Session session = this->user_db_pool->get_connection();

    // 获取请求参数
    std::string db_name = req->database();
    std::string tab_name = req->table();
    // 假设数据库操作
    // ...
    // 设置响应参数
    res->set_success(true);
    res->set_message("Delete successful");
    std::cout << "Database delete successful" << std::endl;

    // 释放数据库连接
    this->user_db_pool->release_connection(std::move(session));
}

/******************************************** 其他工具函数 *****************************************************/
// 读取 数据库配置配置文件，获得数据库连接字符串
std::string DBServerImpl::Read_db_config(lua_State* L, const std::string& file_url)
{
    if(luaL_dofile(L,file_url.c_str()) != LUA_OK) {
        std::cerr << "Error: " << lua_tostring(L,-1) << std::endl;
        lua_pop(L,1);
        return "";
    }

    lua_getglobal(L,"db_config");
    if(!lua_istable(L,-1)) {
        std::cerr << "Error: db_config is not a table" << std::endl;
        lua_pop(L,1);
        return "";
    }

    lua_getfield(L,-1,"mysqlx");
    if(!lua_istable(L,-1)) {
        std::cerr << "Error: mysqlx is not a table" << std::endl;
        lua_pop(L,1);
        return "";
    }

    std::string host,port,username,password,db_name;
    lua_getfield(L,-1,"Host");
    host = lua_tostring(L,-1);
    lua_pop(L,1);

    lua_getfield(L,-1,"Port");
    port = lua_tostring(L,-1);
    lua_pop(L,1);

    lua_getfield(L,-1,"UserName");
    username = lua_tostring(L,-1);
    lua_pop(L,1);

    lua_getfield(L,-1,"Password");
    password = lua_tostring(L,-1);
    lua_pop(L,1);

    lua_getfield(L,-1,"db_name");
    lua_getfield(L,-1,"user_db");
    db_name = lua_tostring(L,-1);
    lua_pop(L,2);

    lua_pop(L,2); // pop mysqlx and db_config

    std::string db_uri = "mysqlx://" + username + ":" + password + "@" + host + ":" + port + "/" + db_name;
    return db_uri;
}
