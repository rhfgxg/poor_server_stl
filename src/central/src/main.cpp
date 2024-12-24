#include "central_server.h"
#include "logger_manager.h" // 引入日志管理器

// 运行服务器
void RunServer(LoggerManager& logger_manager);

// 中心服务器main函数
int main()
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(myproject::ServerType::CENTRAL);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("中心服务器启动"); // 记录启动日志：日志分类, 日志内容

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
