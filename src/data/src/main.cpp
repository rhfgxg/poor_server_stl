#include "./data/data_server.h"
#include "logger_manager.h" // 引入日志管理器

// 运行服务器
void RunServer(LoggerManager& logger_manager,DBConnectionPool& db_pool); 

// 数据库服务器main函数
int main()
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(myproject::ServerType::DATA);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Data_server started"); // 记录启动日志：日志分类, 日志内容

    // 初始化数据库连接池
    DBConnectionPool db_pool("mysqlx://root:159357@localhost:33060",10);

    // 启动服务器
    RunServer(logger_manager,db_pool); // 运行服务器

    // 关闭服务器
    return 0;
}

void RunServer(LoggerManager& logger_manager,DBConnectionPool& db_pool)
{
    DatabaseServerImpl db_server(logger_manager,db_pool); // 数据库rpc服务实现，传入数据库连接池

    grpc::ServerBuilder builder;
    std::string server_address("0.0.0.0:50052");    // 数据库服务器监听50052端口
    builder.AddListeningPort(server_address,grpc::InsecureServerCredentials());
    builder.RegisterService(&db_server);  // 注册服务

    db_server.start_thread_pool(4); // 启动4个线程处理请求

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Listening address: {}",server_address);

    server->Wait();
    db_server.stop_thread_pool(); // 停止线程池
}
