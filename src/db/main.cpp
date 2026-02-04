#include "./server/db_server.h"
#include "logger_manager.h"
#include <iostream>
#include <lua.hpp>

void run_server(LoggerManager& logger_manager);
void read_server_config(std::string& address, std::string& port);

/**
 * @brief 数据库服务器主函数（重构版本）
 */
int main()
{
    try
    {
        // 初始化日志管理器
        LoggerManager logger_manager;
        logger_manager.initialize(rpc_server::ServerType::DB);

        logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("DB_server starting...");

        // 运行服务器
        run_server(logger_manager);

        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error in main: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

void run_server(LoggerManager& logger_manager)
{
    // 读取配置文件
    std::string address = "0.0.0.0";
    std::string port = "50052";
    read_server_config(address, port);

    // 创建服务器实例（BaseServer 会自动管理线程池、注册、心跳）
    DBServerImpl db_server(logger_manager, address, port);

    // 启动服务器（BaseServer 自动处理初始化）
    if (!db_server.start())
    {
        logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->error("Failed to start server");
        return;
    }

    // 构建并启动 gRPC 服务器
    grpc::ServerBuilder builder;
    std::string server_address = address + ":" + port;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&db_server);

    std::unique_ptr<grpc::Server> grpc_server(builder.BuildAndStart());
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Listening on: {}", server_address);

    // 等待服务器关闭
    grpc_server->Wait();

    // 停止服务器（BaseServer 自动处理清理）
    db_server.stop();
}

/**
 * @brief 读取服务器配置文件
 * @param address 服务器地址（输出参数）
 * @param port 服务器端口（输出参数）
 * 
 * 支持多种配置文件路径（按优先级）：
 * 1. config/cfg_db_server.lua (运行时目录的 config/)
 * 2. config/cpp/cfg_server/cfg_db_server.lua (项目根目录)
 * 3. ../config/cpp/cfg_server/cfg_db_server.lua (从 build/ 运行)
 */
void read_server_config(std::string& address, std::string& port)
{
    // 可能的配置文件路径（按优先级排序）
    const char* possible_paths[] = {
        "config/cfg_db_server.lua",                          // build/src/db/config/ (copy_config.sh 复制后)
        "config/cpp/cfg_server/cfg_db_server.lua",           // 从项目根目录运行
        "../config/cpp/cfg_server/cfg_db_server.lua",        // 从 build/ 目录运行
        "../../config/cpp/cfg_server/cfg_db_server.lua",     // 从 build/src/db/ 运行（但没有 config/）
        "../../../config/cpp/cfg_server/cfg_db_server.lua"  // 备用路径
    };

    lua_State* L = nullptr;
    std::string loaded_path;
    bool loaded = false;

    try
    {
        L = luaL_newstate();
        if (!L)
        {
            throw std::runtime_error("Failed to create Lua state");
        }
        luaL_openlibs(L);

        // 尝试多个可能的路径
        for (const char* path : possible_paths)
        {
            if (luaL_dofile(L, path) == LUA_OK)
            {
                loaded = true;
                loaded_path = path;
                break;
            }
            // 清除错误信息，继续尝试下一个路径
            lua_pop(L, 1);
        }

        if (!loaded)
        {
            lua_close(L);
            L = nullptr;
            throw std::runtime_error("Config file not found in any expected location");
        }

        if (!lua_istable(L, -1))
        {
            lua_close(L);
            L = nullptr;
            throw std::runtime_error("Config file must return a table");
        }

        // 读取 host
        lua_getfield(L, -1, "host");
        if (lua_isstring(L, -1))
        {
            address = lua_tostring(L, -1);
        }
        lua_pop(L, 1);

        // 读取 port
        lua_getfield(L, -1, "port");
        if (lua_isinteger(L, -1))
        {
            port = std::to_string(lua_tointeger(L, -1));
        }
        lua_pop(L, 1);

        lua_close(L);
        
        std::cout << "Config loaded from: " << loaded_path << std::endl;
    }
    catch (const std::exception& e)
    {
        if (L)
        {
            lua_close(L);
        }
        std::cerr << "Warning: " << e.what() << std::endl;
        std::cerr << "Using defaults: " << address << ":" << port << std::endl;
    }
}
