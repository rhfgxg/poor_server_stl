#include "./server/central_server.h"
#include "logger_manager.h" // 引入日志管理器

void run_server(LoggerManager& logger_manager); // 运行服务器
void Read_server_config(std::string& address, std::string& port);  // 读取配置文件，获取服务器地址和端口

// 中心服务器main函数
int main()
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(rpc_server::ServerType::CENTRAL);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->info("Central_server started");

    run_server(logger_manager); // 运行服务器

    return 0;
}

void run_server(LoggerManager& logger_manager)
{
    std::string address = "0.0.0.0";    // 默认服务器地址
    std::string port = "50050";
    Read_server_config(address, port);  // 读取服务器配置文件，初始化服务器地址和端口

    CentralServerImpl central_server(logger_manager, address, port);   // 传入日志管理器

    grpc::ServerBuilder builder;
    std::string server_address(address + ":" + port);
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&central_server); // 注册服务

    central_server.start_thread_pool(4); // 启动4个线程处理请求

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());  // 创建服务器

    // 记录监听地址
    logger_manager.getLogger(rpc_server::LogCategory::STARTUP_SHUTDOWN)->info("Listening address: {}", server_address);
    server->Wait(); // 等待请求

    central_server.stop_thread_pool(); // 停止线程池
}

// 读取服务器配置文件，初始化服务器地址和端口
void Read_server_config(std::string& address, std::string& port)
{
    lua_State* L = luaL_newstate();  // 创建lua虚拟机
    luaL_openlibs(L);   // 打开lua标准库

    std::string file_url = "config/central_server_config.lua";  // 配置文件路径

    if(luaL_dofile(L, file_url.c_str()) != LUA_OK)
    {
        lua_close(L);
        throw std::runtime_error("Failed to load config file");
    }

    lua_getglobal(L, "central_server");
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
