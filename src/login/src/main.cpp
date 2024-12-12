#include "login/login_server.h"
#include "server_login.grpc.pb.h"	// 登录服务

#include "../../central/src/central/central_server.h"

#include <iostream>
#include <grpcpp/grpcpp.h>

void RunServer();   // 运行服务器
void RegisterServer(); // 注册服务器
void UnregisterServer(); // 注销服务器

int main() 
{
    // 启动服务器
    RunServer(); // 运行服务器

    // 关闭服务器
    UnregisterServer(); // 注销服务器
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
    RegisterServer();

    server->Wait(); // 等待服务器终止
}

void RegisterServer()
{
    auto channel = grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()); // 链接中心服务器
    auto stub = myproject::CentralServer::NewStub(channel); // 服务存根

    // 请求
    myproject::RegisterServerRequest request;
    request.set_server_name("DataServer");
    request.set_address("127.0.0.1");
    request.set_port(50052);

    // 响应
    myproject::RegisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = stub->RegisterServer(&context, request, &response);

    if (status.ok() && response.success()) {
        std::cout << "服务器注册成功: " << response.message() << std::endl;
    }
    else {
        std::cerr << "服务器注册失败: " << response.message() << std::endl;
    }
}

void UnregisterServer()
{
    auto channel = grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()); // 链接中心服务器
    auto stub = myproject::CentralServer::NewStub(channel); // 服务存根

    // 请求
    myproject::UnregisterServerRequest request;
    request.set_server_name("DataServer");

    // 响应
    myproject::UnregisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = stub->UnregisterServer(&context, request, &response);

    if (status.ok() && response.success()) {
        std::cout << "服务器注销成功: " << response.message() << std::endl;
    }
    else {
        std::cerr << "服务器注销失败: " << response.message() << std::endl;
    }
}