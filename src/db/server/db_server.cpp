#include "db_server.h"
#include <lua.hpp>

// 构造函数 - 继承 BaseServer
DBServerImpl::DBServerImpl(LoggerManager& logger_manager_, const std::string& address, const std::string& port)
    : BaseServer(rpc_server::ServerType::DB, address, port, logger_manager_, 4),  // 4 个线程
      socket_server_running_(false),
      socket_server_fd_(-1),
      socket_port_(50053)  // ✅ Socket 服务器默认端口
{
    // 数据库连接池将在 on_start() 中初始化
}

// 析构函数
DBServerImpl::~DBServerImpl()
{
    stop_socket_server();  // ✅ 停止 Socket 服务器
    log_shutdown("DBServer stopped");
}

// ==================== BaseServer 钩子方法 ====================

bool DBServerImpl::on_start()
{
    log_startup("DBServer initializing...");
    
    // 初始化数据库连接池
    if (!init_db_pools())
    {
        log_startup("Failed to initialize database connection pools");
        return false;
    }
    
    // ✅ 启动 Socket 服务器
    start_socket_server(socket_port_);
    
    log_startup("DBServer initialized successfully");
    return true;
}

void DBServerImpl::on_stop()
{
    log_shutdown("DBServer shutting down...");
    
    // ✅ 停止 Socket 服务器
    stop_socket_server();
    
    // 数据库连接池会自动清理（unique_ptr）
    
    log_shutdown("DBServer shutdown complete");
}

// 初始化数据库连接池
bool DBServerImpl::init_db_pools()
{
    try
    {
        // 可能的配置文件路径（按优先级排序）
        const char* possible_paths[] = {
            "config/cfg_db.lua",                          // build/src/db/config/ (copy_config.sh 复制后)
            "config/cpp/cfg_server/cfg_db.lua",           // 从项目根目录运行
            "../config/cpp/cfg_server/cfg_db.lua",        // 从 build/ 目录运行
            "../../config/cpp/cfg_server/cfg_db.lua",     // 从 build/src/db/ 运行（但没有 config/）
            "../../../config/cpp/cfg_server/cfg_db.lua"  // 备用路径
        };

        lua_State* L = luaL_newstate();
        if (!L)
        {
            log_startup("Failed to create Lua state");
            return false;
        }
        luaL_openlibs(L);

        // 尝试加载配置文件
        bool loaded = false;
        std::string loaded_path;
        
        for (const char* path : possible_paths)
        {
            if (luaL_dofile(L, path) == LUA_OK)
            {
                loaded = true;
                loaded_path = path;
                break;
            }
            lua_pop(L, 1);  // 清除错误信息
        }

        if (!loaded)
        {
            log_startup("Failed to load DB config from any expected location");
            lua_close(L);
            return false;
        }

        log_startup("DB config loaded from: " + loaded_path);

        // 验证配置格式
        if (!lua_istable(L, -1))
        {
            log_startup("DB config is not a table");
            lua_close(L);
            return false;
        }

        // 获取 mysqlx 表
        lua_getfield(L, -1, "mysqlx");
        if (!lua_istable(L, -1))
        {
            log_startup("mysqlx section not found in config");
            lua_close(L);
            return false;
        }

        // Lambda 函数：从配置中读取数据库 URI
        auto read_db_uri = [&](const std::string& db_name) -> std::string
        {
            lua_getfield(L, -1, db_name.c_str());
            if (!lua_istable(L, -1))
            {
                log_startup("Database config not found: " + db_name);
                lua_pop(L, 1);
                return "";
            }

            auto get_field = [&](const char* field_name) -> std::string
            {
                lua_getfield(L, -1, field_name);
                std::string value;
                if (lua_isstring(L, -1))
                {
                    value = lua_tostring(L, -1);
                }
                lua_pop(L, 1);
                return value;
            };

            std::string host = get_field("host");
            std::string port = get_field("port");
            std::string username = get_field("user_name");
            std::string password = get_field("password");

            lua_pop(L, 1);  // 弹出数据库配置表

            if (host.empty() || port.empty() || username.empty() || password.empty())
            {
                log_startup("Incomplete configuration for database: " + db_name);
                return "";
            }

            return "mysqlx://" + username + ":" + password + "@" + host + ":" + port + "/" + db_name;
        };

        // 初始化 poor_users 连接池
        std::string db_uri = read_db_uri("poor_users");
        if (db_uri.empty())
        {
            lua_close(L);
            return false;
        }
        poor_users_pool_ = std::make_unique<DBConnectionPool>(db_uri, 10);
        log_startup("poor_users connection pool initialized");

        // 初始化 poor_net_disk 连接池
        db_uri = read_db_uri("poor_net_disk");
        if (db_uri.empty())
        {
            lua_close(L);
            return false;
        }
        poor_file_pool_ = std::make_unique<DBConnectionPool>(db_uri, 10);
        log_startup("poor_net_disk connection pool initialized");

        // 初始化 poor_hearthstone 连接池
        db_uri = read_db_uri("poor_hearthstone");
        if (db_uri.empty())
        {
            lua_close(L);
            return false;
        }
        hearthstone_pool_ = std::make_unique<DBConnectionPool>(db_uri, 10);
        log_startup("poor_hearthstone connection pool initialized");

        lua_close(L);
        return true;
    }
    catch (const std::exception& e)
    {
        log_startup(std::string("Failed to initialize DB pools: ") + e.what());
        return false;
    }
}

// ==================== gRPC 服务接口 ====================

grpc::Status DBServerImpl::Create(grpc::ServerContext* context [[maybe_unused]], const rpc_server::CreateReq* req, rpc_server::CreateRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_create(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status DBServerImpl::Read(grpc::ServerContext* context [[maybe_unused]], const rpc_server::ReadReq* req, rpc_server::ReadRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_read(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status DBServerImpl::Update(grpc::ServerContext* context [[maybe_unused]], const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_update(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status DBServerImpl::Delete(grpc::ServerContext* context [[maybe_unused]], const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_delete(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status DBServerImpl::Create_table(grpc::ServerContext* context [[maybe_unused]], const rpc_server::CreateTableReq* req, rpc_server::CreateTableRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_create_table(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

// ==================== 数据库操作处理函数 ====================

void DBServerImpl::Handle_create(const rpc_server::CreateReq* req, rpc_server::CreateRes* res)
{
    try
    {
        std::string db_name = req->database();
        std::string tab_name = req->table();
        const auto& data = req->data();

        std::unique_ptr<mysqlx::Session> session = Get_connection(db_name);
        mysqlx::Table table = session->getSchema(db_name).getTable(tab_name);

        std::vector<std::string> columns;
        std::vector<mysqlx::Value> values;
        for (const auto& field : data)
        {
            columns.push_back(field.first);
            values.push_back(field.second);
        }

        table.insert(columns).values(values).execute();

        res->set_success(true);
        res->set_message("Create successful");
        get_logger(poor::LogCategory::DATABASE_OPERATIONS)->info("DB create successful");

        Release_connection(db_name, std::move(session));
    }
    catch (const std::exception& e)
    {
        res->set_success(false);
        res->set_message("Create failed: " + std::string(e.what()));
        get_logger(poor::LogCategory::DATABASE_OPERATIONS)->error("DB create failed: {}", e.what());
    }
}

void DBServerImpl::Handle_read(const rpc_server::ReadReq* req, rpc_server::ReadRes* res)
{
    try
    {
        const std::string& db_name = req->database();
        const std::string& tab_name = req->table();
        const auto& query = req->query();

        std::unique_ptr<mysqlx::Session> session = Get_connection(db_name);
        mysqlx::Table table = session->getSchema(db_name).getTable(tab_name);

        std::string condition;
        for (const auto& q : query)
        {
            if (!condition.empty())
            {
                condition += " AND ";
            }
            condition += q.first + " = '" + q.second + "'";
        }

        mysqlx::RowResult result = table.select("*").where(condition).execute();

        for (mysqlx::Row row : result)
        {
            rpc_server::Result* response_result = res->add_results();
            for (size_t i = 0; i < row.colCount(); ++i)
            {
                std::string column_name = result.getColumn(i).getColumnName();
                std::string column_value;
                
                if (row[i].isNull())
                {
                    column_value = "NULL";
                }
                else if (row[i].getType() == mysqlx::Value::Type::STRING)
                {
                    column_value = row[i].get<std::string>();
                }
                else if (row[i].getType() == mysqlx::Value::Type::VNULL)
                {
                    column_value = "VNULL";
                }
                else if (row[i].getType() == mysqlx::Value::Type::INT64)
                {
                    column_value = std::to_string(row[i].get<int64_t>());
                }
                else if (row[i].getType() == mysqlx::Value::Type::UINT64)
                {
                    column_value = std::to_string(row[i].get<uint64_t>());
                }
                else if (row[i].getType() == mysqlx::Value::Type::FLOAT)
                {
                    column_value = std::to_string(row[i].get<float>());
                }
                else if (row[i].getType() == mysqlx::Value::Type::DOUBLE)
                {
                    column_value = std::to_string(row[i].get<double>());
                }
                else if (row[i].getType() == mysqlx::Value::Type::BOOL)
                {
                    column_value = row[i].get<bool>() ? "true" : "false";
                }
                else if (row[i].getType() == mysqlx::Value::Type::RAW)
                {
                    column_value = "Raw Data";
                }
                else
                {
                    column_value = "Unsupported Type";
                }
                
                response_result->mutable_fields()->insert({column_name, column_value});
            }
        }

        res->set_success(true);
        res->set_message("Query successful");
        get_logger(poor::LogCategory::DATABASE_OPERATIONS)->info("DB read successful");

        Release_connection(db_name, std::move(session));
    }
    catch (const std::exception& e)
    {
        res->set_success(false);
        res->set_message("Query failed: " + std::string(e.what()));
        get_logger(poor::LogCategory::DATABASE_OPERATIONS)->error("DB read failed: {}", e.what());
    }
}

void DBServerImpl::Handle_update(const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res)
{
    try
    {
        std::string db_name = req->database();
        std::string tab_name = req->table();
        const auto& query = req->query();
        const auto& data = req->data();

        std::unique_ptr<mysqlx::Session> session = Get_connection(db_name);
        mysqlx::Table table = session->getSchema(db_name).getTable(tab_name);

        std::string condition;
        for (const auto& q : query)
        {
            if (!condition.empty())
            {
                condition += " AND ";
            }
            condition += q.first + " = '" + q.second + "'";
        }

        mysqlx::TableUpdate update = table.update();
        for (const auto& d : data)
        {
            update.set(d.first, mysqlx::Value(d.second));
        }
        update.where(condition).execute();

        res->set_success(true);
        res->set_message("Update successful");
        get_logger(poor::LogCategory::DATABASE_OPERATIONS)->info("DB update successful");

        Release_connection(db_name, std::move(session));
    }
    catch (const std::exception& e)
    {
        res->set_success(false);
        res->set_message("Update failed: " + std::string(e.what()));
        get_logger(poor::LogCategory::DATABASE_OPERATIONS)->error("DB update failed: {}", e.what());
    }
}

void DBServerImpl::Handle_delete(const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res)
{
    std::string db_name = req->database();
    std::string tab_name = req->table();

    std::unique_ptr<mysqlx::Session> session = Get_connection(db_name);
    mysqlx::Table table = session->getSchema(db_name).getTable(tab_name);

    res->set_success(true);
    res->set_message("Delete successful");

    Release_connection(db_name, std::move(session));
}

void DBServerImpl::Handle_create_table(const rpc_server::CreateTableReq* req, rpc_server::CreateTableRes* res)
{
    try
    {
        std::string db_name = req->database();
        std::string tab_name = req->table();

        std::unique_ptr<mysqlx::Session> session = Get_connection(db_name);

        std::string sql = "CREATE TABLE `" + tab_name + "` (";
        
        for (const auto& field : req->fields())
        {
            sql += "`" + field.name() + "` " + field.type();
            if (field.not_null())
            {
                sql += " NOT NULL";
            }
            if (!field.default_value().empty())
            {
                sql += " DEFAULT " + field.default_value();
            }
            if (field.auto_increment())
            {
                sql += " AUTO_INCREMENT";
            }
            if (!field.comment().empty())
            {
                sql += " COMMENT '" + field.comment() + "'";
            }
            sql += ",";
        }

        for (const auto& constraint : req->constraints())
        {
            if (constraint.type() == "PRIMARY_KEY")
            {
                sql += "PRIMARY KEY (";
                for (const auto& field : constraint.fields())
                {
                    sql += "`" + field + "`,";
                }
                sql.pop_back();
                sql += "),";
            }
            else if (constraint.type() == "UNIQUE")
            {
                sql += "UNIQUE KEY `" + constraint.name() + "` (";
                for (const auto& field : constraint.fields())
                {
                    sql += "`" + field + "`,";
                }
                sql.pop_back();
                sql += "),";
            }
        }
        sql.pop_back();
        sql += ")";

        if (!req->engine().empty())
        {
            sql += " ENGINE = " + req->engine();
        }
        if (!req->charset().empty())
        {
            sql += " CHARACTER SET = " + req->charset();
        }
        if (!req->collation().empty())
        {
            sql += " COLLATE = " + req->collation();
        }
        if (!req->table_comment().empty())
        {
            sql += " COMMENT = '" + req->table_comment() + "'";
        }
        sql += " ROW_FORMAT = Dynamic;";

        session->sql(sql).execute();

        res->set_success(true);
        res->set_message("Table created successfully");
        get_logger(poor::LogCategory::DATABASE_OPERATIONS)->info("DB make table successful");

        Release_connection(db_name, std::move(session));
    }
    catch (const std::exception& e)
    {
        res->set_success(false);
        res->set_message("Create table failed: " + std::string(e.what()));
        get_logger(poor::LogCategory::DATABASE_OPERATIONS)->error("DB make table failed: {}", e.what());
    }
}

// ==================== 数据库连接池工具函数 ====================

std::unique_ptr<mysqlx::Session> DBServerImpl::Get_connection(const std::string& db_name)
{
    if (db_name == "poor_users")
    {
        return poor_users_pool_->Get_connection();
    }
    else if (db_name == "poor_file_hub" || db_name == "poor_net_disk")
    {
        return poor_file_pool_->Get_connection();
    }
    else if (db_name == "hearthstone" || db_name == "poor_hearthstone")
    {
        return hearthstone_pool_->Get_connection();
    }
    else
    {
        throw std::runtime_error("Unknown database name: " + db_name);
    }
}

void DBServerImpl::Release_connection(const std::string& db_name, std::unique_ptr<mysqlx::Session> session)
{
    if (db_name == "poor_users")
    {
        poor_users_pool_->Release_connection(std::move(session));
    }
    else if (db_name == "poor_file_hub" || db_name == "poor_net_disk")
    {
        poor_file_pool_->Release_connection(std::move(session));
    }
    else if (db_name == "hearthstone" || db_name == "poor_hearthstone")
    {
        hearthstone_pool_->Release_connection(std::move(session));
    }
    else
    {
        throw std::runtime_error("Unknown database name: " + db_name);
    }
}

// ==================== Socket 服务器实现 ====================
// 添加到 db_server.cpp 末尾

void DBServerImpl::start_socket_server(int socket_port)
{
    if(socket_server_running_.load())
    {
        log_activity("Socket server already running");
        return;
    }

    socket_port_ = socket_port;
    socket_server_running_.store(true);

    // 启动 Socket 服务器线程
    socket_server_thread_ = std::thread(&DBServerImpl::socket_server_thread, this);

    log_activity("Socket server started on port: " + std::to_string(socket_port_));
}

void DBServerImpl::stop_socket_server()
{
    if(!socket_server_running_.load())
    {
        return;
    }

    socket_server_running_.store(false);

    // 关闭服务器 socket
    if(socket_server_fd_ != -1)
    {
        #ifdef _WIN32
        closesocket(socket_server_fd_);
        #else
        close(socket_server_fd_);
        #endif
        socket_server_fd_ = -1;
    }

    // 等待线程结束
    if(socket_server_thread_.joinable())
    {
        socket_server_thread_.join();
    }

    log_activity("Socket server stopped");
}

void DBServerImpl::socket_server_thread()
{
    log_activity("Socket server thread started");

    #ifdef _WIN32
    // Windows: 初始化 WinSock
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        log_activity("WSAStartup failed");
        return;
    }
    #endif

    // 创建 socket
    socket_server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_server_fd_ < 0)
    {
        log_activity("Failed to create socket");
        #ifdef _WIN32
        WSACleanup();
        #endif
        return;
    }

    // 设置 socket 选项：允许端口重用
    int opt = 1;
    #ifdef _WIN32
    setsockopt(socket_server_fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    #else
    setsockopt(socket_server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif

    // 绑定地址和端口
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(socket_port_);

    if(bind(socket_server_fd_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        log_activity("Failed to bind socket on port: " + std::to_string(socket_port_));
        #ifdef _WIN32
        closesocket(socket_server_fd_);
        WSACleanup();
        #else
        close(socket_server_fd_);
        #endif
        return;
    }

    // 监听连接
    if(listen(socket_server_fd_, 10) < 0)
    {
        log_activity("Failed to listen on socket");
        #ifdef _WIN32
        closesocket(socket_server_fd_);
        WSACleanup();
        #else
        close(socket_server_fd_);
        #endif
        return;
    }

    log_activity("Socket server listening on port: " + std::to_string(socket_port_));

    // 接受客户端连接
    while(socket_server_running_.load())
    {
        sockaddr_in client_addr;
        #ifdef _WIN32
        int client_len = sizeof(client_addr);
        #else
        socklen_t client_len = sizeof(client_addr);
        #endif

        int client_socket = accept(socket_server_fd_, (sockaddr*)&client_addr, &client_len);

        if(client_socket < 0)
        {
            if(socket_server_running_.load())
            {
                log_activity("Failed to accept client connection");
            }
            continue;
        }

        // 获取客户端 IP
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        log_activity("Socket client connected: " + std::string(client_ip));

        // 在线程池中处理客户端请求
        submit_task([this, client_socket]() {
            handle_socket_client(client_socket);
        });
    }

    #ifdef _WIN32
    closesocket(socket_server_fd_);
    WSACleanup();
    #else
    close(socket_server_fd_);
    #endif

    log_activity("Socket server thread exiting");
}

// Socket 客户端处理函数 - Part 2
// 添加到 db_server.cpp

void DBServerImpl::handle_socket_client(int client_socket)
{
    try
    {
        // 1. 读取请求长度（4 字节，网络字节序）
        uint32_t request_length = 0;
        int bytes_read = recv(client_socket, (char*)&request_length, 4, 0);

        if(bytes_read != 4)
        {
            log_activity("Failed to read request length");
            #ifdef _WIN32
            closesocket(client_socket);
            #else
            close(client_socket);
            #endif
            return;
        }

        // 转换为主机字节序
        request_length = ntohl(request_length);

        if(request_length == 0 || request_length > 1024 * 1024)  // 限制 1MB
        {
            log_activity("Invalid request length: " + std::to_string(request_length));
            #ifdef _WIN32
            closesocket(client_socket);
            #else
            close(client_socket);
            #endif
            return;
        }

        // 2. 读取 JSON 请求体
        std::vector<char> buffer(request_length);
        int total_read = 0;

        while(total_read < (int)request_length)
        {
            int n = recv(client_socket, buffer.data() + total_read, request_length - total_read, 0);
            if(n <= 0)
            {
                log_activity("Failed to read request body");
                #ifdef _WIN32
                closesocket(client_socket);
                #else
                close(client_socket);
                #endif
                return;
            }
            total_read += n;
        }

        std::string json_request(buffer.begin(), buffer.end());
        log_activity("Socket request received: " + json_request.substr(0, std::min<size_t>(100, json_request.size())));

        // 3. 处理请求
        std::string json_response = handle_socket_request(json_request);

        // 4. 发送响应长度
        uint32_t response_length = htonl(static_cast<uint32_t>(json_response.size()));
        send(client_socket, (const char*)&response_length, 4, 0);

        // 5. 发送响应体
        int total_sent = 0;
        while(total_sent < (int)json_response.size())
        {
            int n = send(client_socket, json_response.data() + total_sent, json_response.size() - total_sent, 0);
            if(n <= 0)
            {
                log_activity("Failed to send response");
                break;
            }
            total_sent += n;
        }

        log_activity("Socket response sent");
    } catch(const std::exception& e)
    {
        log_activity("Socket client error: " + std::string(e.what()));
    }

    // 关闭客户端连接
    #ifdef _WIN32
    closesocket(client_socket);
    #else
    close(client_socket);
    #endif
}

// JSON 请求/响应处理 - Part 3
// 添加到 db_server.cpp

std::string DBServerImpl::handle_socket_request(const std::string& json_request)
{
    try
    {
        // 简单的 JSON 解析（手动）
        // 生产环境建议使用 rapidjson 或其他 JSON 库

        // ✅ 修复：提取 method（在第一个双引号到第二个双引号之间）
        std::string method;
        size_t method_pos = json_request.find("\"method\":");
        if(method_pos != std::string::npos)
        {
            size_t start = json_request.find("\"", method_pos + 9);  // ✅ 从 "method": 后开始找
            size_t end = json_request.find("\"", start + 1);
            if(start != std::string::npos && end != std::string::npos)
            {
                method = json_request.substr(start + 1, end - start - 1);
            }
        }

        // ✅ 修复：提取 database
        std::string database;
        size_t db_pos = json_request.find("\"database\":");
        if(db_pos != std::string::npos)
        {
            size_t start = json_request.find("\"", db_pos + 11);  // ✅ 从 "database": 后开始找
            size_t end = json_request.find("\"", start + 1);
            if(start != std::string::npos && end != std::string::npos)
            {
                database = json_request.substr(start + 1, end - start - 1);
            }
        }

        // ✅ 修复：提取 table
        std::string table;
        size_t table_pos = json_request.find("\"table\":");
        if(table_pos != std::string::npos)
        {
            size_t start = json_request.find("\"", table_pos + 8);  // ✅ 从 "table": 后开始找
            size_t end = json_request.find("\"", start + 1);
            if(start != std::string::npos && end != std::string::npos)
            {
                table = json_request.substr(start + 1, end - start - 1);
            }
        }

        log_activity("Socket request - Method: " + method + ", Database: " + database + ", Table: " + table);

        // 根据 method 调用相应的处理函数
        if(method == "Read")
        {
            // 构造 gRPC 请求
            rpc_server::ReadReq req;
            req.set_database(database);
            req.set_table(table);

            // 提取 query 参数
            size_t query_pos = json_request.find("\"query\":{");
            if(query_pos != std::string::npos)
            {
                // ✅ 优化：提取完整的 query 对象
                size_t query_start = query_pos + 8;  // "query":{
                size_t brace_count = 1;
                size_t query_end = query_start + 1;
                
                while(query_end < json_request.size() && brace_count > 0)
                {
                    if(json_request[query_end] == '{') brace_count++;
                    else if(json_request[query_end] == '}') brace_count--;
                    query_end++;
                }
                
                std::string query_json = json_request.substr(query_start, query_end - query_start);
                
                // 解析 query 字段（通用版本）
                auto extract_query_field = [&](const std::string& field_name) -> std::string
                {
                    size_t pos = query_json.find("\"" + field_name + "\":");
                    if(pos != std::string::npos)
                    {
                        size_t start = query_json.find("\"", pos + field_name.length() + 3);
                        if(start != std::string::npos)
                        {
                            size_t end = query_json.find("\"", start + 1);
                            if(end != std::string::npos)
                            {
                                return query_json.substr(start + 1, end - start - 1);
                            }
                        }
                    }
                    return "";
                };
                
                // 提取所有可能的查询字段
                std::vector<std::string> query_fields = {"player_id", "user_id", "nickname"};
                
                for(const auto& field : query_fields)
                {
                    std::string value = extract_query_field(field);
                    if(!value.empty())
                    {
                        (*req.mutable_query())[field] = value;
                        log_activity("  Query field: " + field + " = " + value);
                    }
                }
            }

            // 调用现有的 gRPC 处理函数
            rpc_server::ReadRes res;
            Handle_read(&req, &res);

            // 构造 JSON 响应
            std::vector<std::map<std::string, std::string>> results;
            for(const auto& result : res.results())
            {
                std::map<std::string, std::string> row;
                for(const auto& field : result.fields())
                {
                    row[field.first] = field.second;
                }
                results.push_back(row);
            }

            return build_json_response(res.success(), res.message(), results);
        } 
        else if(method == "Create")
        {
            // 构造 gRPC 请求
            rpc_server::CreateReq req;
            req.set_database(database);
            req.set_table(table);

            // ✅ 提取 data 参数
            size_t data_pos = json_request.find("\"data\":{");
            if(data_pos != std::string::npos)
            {
                // 找到 data 对象的结束位置
                size_t data_start = data_pos + 7;  // "data":{
                size_t brace_count = 1;
                size_t data_end = data_start + 1;
                
                // 简单的括号匹配找到 data 对象的结束
                while(data_end < json_request.size() && brace_count > 0)
                {
                    if(json_request[data_end] == '{') brace_count++;
                    else if(json_request[data_end] == '}') brace_count--;
                    data_end++;
                }
                
                // 提取 data 部分的 JSON
                std::string data_json = json_request.substr(data_start, data_end - data_start);
                
                // 解析常见字段
                auto extract_field = [&](const std::string& field_name) -> std::string
                {
                    size_t pos = data_json.find("\"" + field_name + "\":");
                    if(pos != std::string::npos)
                    {
                        size_t start = data_json.find("\"", pos + field_name.length() + 3);
                        if(start != std::string::npos)
                        {
                            size_t end = data_json.find("\"", start + 1);
                            if(end != std::string::npos)
                            {
                                return data_json.substr(start + 1, end - start - 1);
                            }
                        }
                    }
                    return "";
                };
                
                // 提取所有字段（移除 level 和 exp）
                std::vector<std::string> fields = {
                    "player_id", "user_id", "nickname", 
                    "gold", "arcane_dust", "last_login"
                };
                
                for(const auto& field : fields)
                {
                    std::string value = extract_field(field);
                    if(!value.empty())
                    {
                        (*req.mutable_data())[field] = value;
                        log_activity("  Extracted field: " + field + " = " + value);
                    }
                }
            }

            // 调用现有的 gRPC 处理函数
            rpc_server::CreateRes res;
            Handle_create(&req, &res);

            return build_json_response(res.success(), res.message());
        } 
        else
        {
            return build_json_response(false, "Unknown method: " + method);
        }
    } catch(const std::exception& e)
    {
        return build_json_response(false, "Request processing error: " + std::string(e.what()));
    }
}

std::string DBServerImpl::build_json_response(bool success, const std::string& message, const std::vector<std::map<std::string, std::string>>& results)
{
    std::string json = "{";
    json += "\"success\":" + std::string(success ? "true" : "false");
    json += ",\"message\":\"" + message + "\"";

    if(!results.empty())
    {
        json += ",\"results\":[";
        for(size_t i = 0; i < results.size(); ++i)
        {
            if(i > 0) json += ",";
            json += "{";

            bool first_field = true;
            for(const auto& field : results[i])
            {
                if(!first_field) json += ",";
                json += "\"" + field.first + "\":\"" + field.second + "\"";
                first_field = false;
            }

            json += "}";
        }
        json += "]";
    }

    json += "}";
    return json;
}
