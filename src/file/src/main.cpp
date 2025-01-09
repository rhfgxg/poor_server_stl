#include "file/file_server.h"
#include "logger_manager.h" // 引入日志管理器

void RunServer(LoggerManager& logger_manager);  // 运行服务器

// 文件服务器main函数
int main()
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(rpc_server::ServerType::FILE);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("File_server started"); // 记录启动日志：日志分类, 日志内容

    RunServer(logger_manager); // 运行服务器

    return 0; // 返回0表示程序正常结束
}

// 运行服务器
void RunServer(LoggerManager& logger_manager)
{
    FileServerImpl file_server(logger_manager); // 网关服务器实现

    grpc::ServerBuilder builder; // gRPC服务器构建器
    std::string server_address("0.0.0.0:50055"); // 文件服务器监听50055端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&file_server); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // 构建并启动服务器
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Listening address: {}", server_address);

    file_server.start_thread_pool(4); // 启动4个线程处理请求

    server->Wait(); // 等待服务器终止

    file_server.stop_thread_pool(); // 停止线程池
}
