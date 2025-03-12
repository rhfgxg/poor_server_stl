#include "./server/battle_server.h"
#include "logger_manager.h" // 引入日志管理器

void RunServer(LoggerManager& logger_manager);// 运行服务器
void Read_server_config(std::string& address, std::string& port);   // 读取服务器配置文件，初始化服务器地址和端口

// 逻辑服务器
int main()
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(rpc_server::ServerType::BATTLE);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Battle server started"); // 记录启动日志：日志分类, 日志内容

    RunServer(logger_manager); // 运行服务器

    // 关闭服务器
    return 0; // 返回0表示程序正常结束
}

void RunServer(LoggerManager& logger_manager)
{
    std::string address = "127.0.0.1";   // 默认服务器地址和端口
    std::string port = "50056";
    Read_server_config(address, port);

    BattleServerImpl battle_server(logger_manager, address, port);

    grpc::ServerBuilder builder;
    std::string server_address(address + ":" + port);
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&battle_server); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // 构建并启动服务器
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Listening address: {}", server_address);

    battle_server.start_thread_pool(4); // 启动4个线程处理请求

    server->Wait(); // 等待服务器终止
    battle_server.stop_thread_pool(); // 停止线程池
}

// 读取服务器配置文件，初始化服务器地址和端口
void Read_server_config(std::string& address, std::string& port)
{
    lua_State* L = luaL_newstate();  // 创建lua虚拟机
    luaL_openlibs(L);   // 打开lua标准库

    std::string file_url = "config/cfg_battle_server.lua";  // 配置文件路径

    if(luaL_dofile(L, file_url.c_str()) != LUA_OK)
    {
        lua_close(L);
        throw std::runtime_error("Failed to load config file");
    }

    lua_getglobal(L, "battle_server");
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
