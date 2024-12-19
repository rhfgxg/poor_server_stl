#include "central_server.h"
#include "logger_manager.h" // 引入日志管理器
#include <iostream>

// 运行服务器
void RunServer(LoggerManager& logger_manager);

int main()
{
    // 初始化日志管理器
    LoggerManager logger_manager;
    logger_manager.initialize("central_server");

    // 记录启动日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("主模块启动");

    RunServer(logger_manager); // 运行服务器

    return 0;
}

void RunServer(LoggerManager& logger_manager)
{
    CentralServerImpl central_server(logger_manager);   // 传入日志管理器

    grpc::ServerBuilder builder;
    std::string server_address("0.0.0.0:50050"); // 中心服务器监听50050端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&central_server); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("CentralServer 启动，监听地址: {}", server_address);
    server->Wait();
}