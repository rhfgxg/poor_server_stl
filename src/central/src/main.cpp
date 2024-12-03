#include <iostream>
#include "central/central_server.h"

// 运行服务器
void RunServer();

int main()
{
	str::cout << "主模块";
    RunServer(); // 运行服务器

	return 0;
}

void RunServer()
{
    CentralServerImpl central_server; // 登录服务实现

    grpc::ServerBuilder builder; // gRPC服务器构建器
    std::string server_address("0.0.0.0:50050"); // 服务器监听50050端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&central_server); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // 构建并启动服务器
    std::cout << "LoginServer start..." << std::endl; // 输出服务器运行信息
    server->Wait(); // 等待服务器终止
}