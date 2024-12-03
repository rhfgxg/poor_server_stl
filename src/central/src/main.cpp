#include "central/central_server.h"
#include <iostream>

// 运行服务器
void RunServer();

int main()
{
	std::cout << "主模块\n";
    RunServer(); // 运行服务器

	return 0;
}

void RunServer()
{
    CentralServerImpl central_server;

    grpc::ServerBuilder builder;
	std::string server_address("0.0.0.0:50050");// 中心服务器监听50050端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&central_server); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "CentealServer start..." << std::endl;
    server->Wait();
}