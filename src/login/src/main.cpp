#include "login/login_server.h"

#include <iostream>

void RunServer();   // 运行服务器

int main() 
{
    // 启动服务器
    RunServer(); // 运行服务器

    // 关闭服务器
    return 0; // 返回0表示程序正常结束
}

void RunServer()
{
    LoginServerImpl login_server; // 登录服务实现

    grpc::ServerBuilder builder; // gRPC服务器构建器
    std::string server_address("0.0.0.0:50053"); // 登录服务器监听50053端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&login_server); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // 构建并启动服务器
    std::cout << "LoginServer start..." << std::endl; // 输出服务器运行信息

    // 注册服务器
	login_server.register_server(); // 注册服务器

    server->Wait(); // 等待服务器终止
	// 注销服务器
	login_server.unregister_server(); // 注销服务器
}
