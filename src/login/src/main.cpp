#include "login/login_server.h"
#include "logger_manager.h" // 引入日志管理器

// 运行服务器
void RunServer(LoggerManager& logger_manager);

int main() 
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(myproject::ServerType::LOGIN);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("中心服务器启动"); // 记录启动日志：日志分类, 日志内容

    RunServer(logger_manager); // 运行服务器

    // 关闭服务器
    return 0; // 返回0表示程序正常结束
}

void RunServer(LoggerManager& logger_manager)
{
    LoginServerImpl login_server(logger_manager); // 登录服务实现

    grpc::ServerBuilder builder; // gRPC服务器构建器
    std::string server_address("0.0.0.0:50053"); // 登录服务器监听50053端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&login_server); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // 构建并启动服务器
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("LoginServer 启动，监听地址: {}",server_address);

    // 注册服务器
	login_server.register_server(); // 注册服务器

    server->Wait(); // 等待服务器终止
	// 注销服务器
	login_server.unregister_server(); // 注销服务器
}
