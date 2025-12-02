#include "db_server.h"
#include <lua.hpp>

// 构造函数 - 继承 BaseServer
DBServerImpl::DBServerImpl(LoggerManager& logger_manager_, const std::string& address, const std::string& port)
    : BaseServer(rpc_server::ServerType::DB, address, port, logger_manager_, 4)  // 4 个线程
{
    // 数据库连接池将在 on_start() 中初始化
}

// 析构函数
DBServerImpl::~DBServerImpl()
{
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
    
    log_startup("DBServer initialized successfully");
    return true;
}

void DBServerImpl::on_stop()
{
    log_shutdown("DBServer shutting down...");
    
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
