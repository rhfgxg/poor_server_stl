#include "./data/data_server.h"
#include "logger_manager.h" // 引入日志管理器

// 运行服务器
void run_server(LoggerManager& logger_manager);

// 数据库服务器main函数
int main()
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(rpc_server::ServerType::DATA);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Data_server started"); // 记录启动日志：日志分类, 日志内容

    // 启动服务器
    run_server(logger_manager); // 运行服务器

    // 关闭服务器
    return 0;
}

void run_server(LoggerManager& logger_manager)
{// 相关注释请参考 /src/central/src/main.cpp/run_server()
    DatabaseServerImpl db_server(logger_manager);  // 传入日志管理器和数据库连接池

    grpc::ServerBuilder builder;
    std::string server_address("0.0.0.0:50052");
    builder.AddListeningPort(server_address,grpc::InsecureServerCredentials());
    builder.RegisterService(&db_server);

    db_server.start_thread_pool(4);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Listening address: {}",server_address);

    server->Wait();
    db_server.stop_thread_pool();
}
