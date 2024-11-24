#include <iostream>
#include <grpcpp/grpcpp.h>
#include "./login/login_server.h"

// 运行服务器
void RunServer();

int main() 
{
    RunServer(); // 运行服务器
    return 0; // 返回0表示程序正常结束
}

void RunServer()
{
    LoginServiceImpl service; // 登录服务实现

    grpc::ServerBuilder builder; // gRPC服务器构建器
    std::string server_address("0.0.0.0:50051"); // 服务器监听50051端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&service); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // 构建并启动服务器
    std::cout << "登录服务器正在运行..." << std::endl; // 输出服务器运行信息
    server->Wait(); // 等待服务器终止
}