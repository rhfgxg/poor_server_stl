#include "./server/central_server.h"
#include "logger_manager.h"
#include <iostream>
#include <lua.hpp>

void run_server(LoggerManager& logger_manager);

/**
 * @brief 中心服务器主函数
 */
int main()
{
    try
    {
        // 初始化日志管理器
        LoggerManager logger_manager;
        logger_manager.initialize(rpc_server::ServerType::CENTRAL);

        logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Central_server starting...");

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

    // 默认地址和端口
    std::string address = "0.0.0.0";
    std::string port = "50050";
    
    try
    {
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);

        if (luaL_dofile(L, "config/cfg_central_server.lua") == LUA_OK)
        {
            if (lua_istable(L, -1))
            {
                lua_getfield(L, -1, "host");
                if (lua_isstring(L, -1))
                {
                    address = lua_tostring(L, -1);
                }
                lua_pop(L, 1);

                lua_getfield(L, -1, "port");
                if (lua_isinteger(L, -1))
                {
                    port = std::to_string(lua_tointeger(L, -1));
                }
                lua_pop(L, 1);
            }
        }
        lua_close(L);
    }
    catch (const std::exception& e)
    {
        logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->warn(
            "Failed to read config, using defaults: {}", e.what());
    }

    // 创建服务器实例（BaseServer 会自动初始化线程池）
    CentralServerImpl central_server(logger_manager, address, port);

    // 启动服务器（BaseServer 会自动启动线程池、注册等）
    if (!central_server.start())
    {
        logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->error("Failed to start server");
        return;
    }

    // 构建并启动 gRPC 服务器
    grpc::ServerBuilder builder;
    std::string server_address = address + ":" + port;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&central_server);

    std::unique_ptr<grpc::Server> grpc_server(builder.BuildAndStart());
    logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN)->info("Listening on: {}", server_address);

    // 等待服务器关闭
    grpc_server->Wait();

    // 停止服务器（BaseServer 会自动注销、停止线程池等）
    central_server.stop();
}
