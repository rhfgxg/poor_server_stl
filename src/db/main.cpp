#include "./server/db_server.h"
#include "logger_manager.h" // 引入日志管理器

void run_server(LoggerManager& logger_manager); // 运行服务器
void Read_server_config(std::string& address, std::string& port);  // 读取配置文件，获取服务器地址和端口

// 数据库服务器main函数
int main()
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(rpc_server::ServerType::DB);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("DB_server started"); // 记录启动日志：日志分类, 日志内容

    // 启动服务器
    run_server(logger_manager); // 运行服务器

    // 关闭服务器
    return 0;
}

void run_server(LoggerManager& logger_manager)
{// 相关注释请参考 /src/central/src/main.cpp/run_server()

    std::string address = "0.0.0.0";    // 默认服务器地址
    std::string port = "50052";
    Read_server_config(address, port);  // 读取服务器配置文件，初始化服务器地址和端口

    DBServerImpl db_server(logger_manager, address, port);  // 传入日志管理器和数据库连接池

    grpc::ServerBuilder builder;
    std::string server_address(address + ":" + port);
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&db_server);

    db_server.start_thread_pool(4);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Listening address: {}", server_address);

    server->Wait();
    db_server.stop_thread_pool();
}

// 读取服务器配置文件，初始化服务器地址和端口
void Read_server_config(std::string& address, std::string& port)
{
    try
    {
        lua_State* L = luaL_newstate();  // 创建lua虚拟机
        luaL_openlibs(L);   // 打开lua标准库

        std::string file_url = "config/cfg_db_server.lua";  // 配置文件路径

        if(luaL_dofile(L, file_url.c_str()) != LUA_OK)
        {
            lua_close(L);
            throw std::runtime_error("Failed to load config file");
        }

        // 获取返回值（配置表）
        if(!lua_istable(L, -1))
        {
            lua_close(L);
            throw std::runtime_error("Invalid config format");
        }

        lua_getfield(L, -1, "host");
        if(!lua_isstring(L, -1))
        {
            lua_close(L);
            throw std::runtime_error("Invalid host format");
        }
        address = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "port");
        if(!lua_isinteger(L, -1))
        {
            lua_close(L);
            throw std::runtime_error("Invalid port format");
        }
        port = std::to_string(lua_tointeger(L, -1));
        lua_pop(L, 1);

        lua_close(L);   // 关闭lua虚拟机
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error reading config file: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
