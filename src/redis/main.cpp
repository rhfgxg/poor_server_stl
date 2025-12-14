#include "redis_server.h"
#include "logger_manager.h"
#include <csignal>
#include <memory>
#include <iostream>

std::unique_ptr<RedisServer> g_redis_server;

void signal_handler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
        if (g_redis_server)
        {
            g_redis_server->stop();
        }
    }
}

int main(int argc [[maybe_unused]], char* argv[] [[maybe_unused]])
{
    try
    {
        // 注册信号处理
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        
        // 创建日志管理器
        LoggerManager logger_manager;
        logger_manager.initialize(rpc_server::ServerType::REDIS);
        
        auto logger = logger_manager.getLogger(poor::LogCategory::STARTUP_SHUTDOWN);
        logger->info("Redis Server starting...");
        
        // 创建 Redis 服务器
        g_redis_server = std::make_unique<RedisServer>(logger_manager, "0.0.0.0", "8003");
        
        // 启动服务器
        if (!g_redis_server->start())
        {
            std::cerr << "Failed to start Redis server" << std::endl;
            logger->error("Failed to start Redis server");
            return 1;
        }
        
        logger->info("Redis server started on port 8003");
        std::cout << "Redis server started on port 8003" << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl;
        
        // 等待停止信号
        g_redis_server->wait_for_shutdown();
        
        logger->info("Redis server stopped");
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
