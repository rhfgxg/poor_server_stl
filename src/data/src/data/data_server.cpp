#include "data_server.h"

DatabaseServerImpl::DatabaseServerImpl(LoggerManager& logger_manager_):
    server_type(myproject::ServerType::DATA),    // 服务器类型
    logger_manager(logger_manager_),    // 日志管理器
    central_stub(myproject::CentralServer::NewStub(grpc::CreateChannel("localhost:50050",grpc::InsecureChannelCredentials()))) // 中心服务器存根
{
    lua_State* L = luaL_newstate();  // 创建lua虚拟机
    luaL_openlibs(L);   // 打开lua标准库

    read_server_config(L,"config/db_server_config.lua"); // 读取服务器配置文件，初始化服务器地址和端口

    // 读取配置文件并初始化数据库连接池
    std::string db_uri = read_db_config(L, "config/db_config.lua");
    user_db_pool = std::make_unique<DBConnectionPool>(db_uri,10);   // 初始化数据库连接池

    lua_close(L);   // 关闭lua虚拟机

    register_server(); // 注册服务器

    // 启动定时任务
    // 定时向中心服务器发送心跳包
    std::thread(&DatabaseServerImpl::send_heartbeat,this).detach();
}

DatabaseServerImpl::~DatabaseServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    unregister_server(); // 注销服务器

    // 记录关闭日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("DatabaseServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

/*************************************** 多线程工具函数 *****************************************************************/
// 启动线程池
void DatabaseServerImpl::start_thread_pool(int num_threads)
{// 相关注释请参考 /src/central/src/central/central_server.cpp/start_thread_pool()
    for(int i = 0; i < num_threads; ++i)
    {
        thread_pool.emplace_back(&DatabaseServerImpl::worker_thread,this);
    }
}

// 停止线程池
void DatabaseServerImpl::stop_thread_pool()
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
    // 清空任务队列
    std::queue<std::function<void()>> empty;
    std::swap(task_queue,empty);
}

// 线程池工作函数
void DatabaseServerImpl::worker_thread()
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
// 注册服务器
void DatabaseServerImpl::register_server()
{
    // 创建请求
    myproject::RegisterServerRequest request;
    request.set_server_type(this->server_type);
    request.set_address(this->server_address);
    request.set_port(this->server_port);
    // 创建响应
    myproject::RegisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->RegisterServer(&context, request, &response);

    if(status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Database server registered successfully: {} {}",this->server_address,this->server_port);
    }
    else
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Database server registration failed: {} {}",this->server_address,this->server_port);
    }
}

// 注销服务器
void DatabaseServerImpl::unregister_server()
{
    // 请求
    myproject::UnregisterServerRequest request;
    request.set_server_type(myproject::ServerType::DATA);
    request.set_address("localhost");
    request.set_port("50052");

    // 响应
    myproject::UnregisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->UnregisterServer(&context,request,&response);

    if(status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Server unregistration successful: {}", response.message());
    }
    else
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Server unregistration failed:  {}",response.message());
    }
}

/*************************************** 定时任务 *********************************************************/
// 定时任务：发送心跳包
void DatabaseServerImpl::send_heartbeat()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒发送一次心跳包

        myproject::HeartbeatRequest request;
        myproject::HeartbeatResponse response;
        grpc::ClientContext context;

        request.set_server_type(myproject::ServerType::DATA);
        request.set_address("127.0.0.1"); // 设置服务器ip
        request.set_port("50052"); // 设置服务器端口

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
// 添加
grpc::Status DatabaseServerImpl::Create(grpc::ServerContext* context, const myproject::CreateRequest* request,  myproject::CreateResponse* response)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this, request, response] {
            handle_create(request, response);
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务
    return grpc::Status::OK;
}

// 查询
grpc::Status DatabaseServerImpl::Read(grpc::ServerContext* context, const myproject::ReadRequest* request, myproject::ReadResponse* response)
{
    // 深拷贝请求和响应对象
    auto request_copy = std::make_shared<myproject::ReadRequest>(*request);
    auto response_copy = std::make_shared<myproject::ReadResponse>();
    // 创建 promise 和 future
    std::promise<void> task_promise;
    std::future<void> task_future = task_promise.get_future();

    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this,request_copy, response_copy, &task_promise] {
            handle_read(request_copy.get(), response_copy.get());
            task_promise.set_value();  // 设置 promise 的值，通知主线程任务完成
        });
    }

    queue_cv.notify_one();  // 通知线程池有新任务

    // 等待任务完成
    task_future.get();

    // 将响应结果复制回原响应对象
    *response = *response_copy;

    return grpc::Status::OK;
}

// 更新
grpc::Status DatabaseServerImpl::Update(grpc::ServerContext* context, const myproject::UpdateRequest* request, myproject::UpdateResponse* response)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this, request, response] {
            handle_update(request, response);
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务
    return grpc::Status::OK;
}

// 删除
grpc::Status DatabaseServerImpl::Delete(grpc::ServerContext* context, const myproject::DeleteRequest* request, myproject::DeleteResponse* response)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this, request, response] {
            handle_delete(request, response);
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务
    return grpc::Status::OK;
}

/**************************************** grpc接口工具函数 **************************************************************************/
// 处理数据库操作的函数
// 添加
void DatabaseServerImpl::handle_create(const myproject::CreateRequest* request, myproject::CreateResponse* response)
{
    // 获取数据库连接
    mysqlx::Session session = user_db_pool->get_connection();

    // 获取请求参数
    std::string db_name = request->database();
    std::string tab_name = request->table();
    // 假设数据库操作
    // ...
    // 设置响应参数
    response->set_success(true);
    response->set_message("Create successful");
    std::cout << "Database created successfully" << std::endl;

    // 释放数据库连接
    user_db_pool->release_connection(std::move(session));
}

// 查询
void DatabaseServerImpl::handle_read(const myproject::ReadRequest* request, myproject::ReadResponse* response)
{
    // 获取数据库连接
    mysqlx::Session session = user_db_pool->get_connection();

    // 获取请求的 query 参数
    const std::string& database = request->database();
    const std::string& table = request->table();
    const auto& query = request->query();

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
        myproject::Result* response_result = response->add_results();
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

    logger_manager.getLogger(LogCategory::DATABASE_OPERATIONS)->info("Database query operation successful: Database: {}", database);

    response->set_success(true);
    response->set_message("Query successful");

    // 释放数据库连接
    user_db_pool->release_connection(std::move(session));
}

// 更新
void DatabaseServerImpl::handle_update(const myproject::UpdateRequest* request, myproject::UpdateResponse* response)
{
    // 获取数据库连接
    mysqlx::Session session = user_db_pool->get_connection();

    // 获取请求参数
    std::string db_name = request->database();
    std::string tab_name = request->table();
    // 假设数据库操作
    // ...
    // 设置响应参数
    response->set_success(true);
    response->set_message("Update successful");
    std::cout << "Database update successful" << std::endl;

    // 释放数据库连接
    user_db_pool->release_connection(std::move(session));
}

// 删除
void DatabaseServerImpl::handle_delete(const myproject::DeleteRequest* request, myproject::DeleteResponse* response)
{
    // 获取数据库连接
    mysqlx::Session session = user_db_pool->get_connection();

    // 获取请求参数
    std::string db_name = request->database();
    std::string tab_name = request->table();
    // 假设数据库操作
    // ...
    // 设置响应参数
    response->set_success(true);
    response->set_message("Delete successful");
    std::cout << "Database delete successful" << std::endl;

    // 释放数据库连接
    user_db_pool->release_connection(std::move(session));
}

/******************************************** 其他工具函数 *****************************************************/
// 读取服务器配置文件，初始化服务器地址和端口
void DatabaseServerImpl::read_server_config(lua_State* L, const std::string& file_url)
{
    if(luaL_dofile(L,file_url.c_str()) != LUA_OK)
    {
        lua_close(L);
        throw std::runtime_error("Failed to load config file");
    }

    lua_getglobal(L,"db_server");
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
}

// 读取 数据库配置配置文件，获得数据库连接字符串
std::string DatabaseServerImpl::read_db_config(lua_State* L, const std::string& file_url)
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
