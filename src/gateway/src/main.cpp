#include "./gateway/gateway_server.h"

#include <iostream>
#include <grpcpp/grpcpp.h>

void RunServer();   // 运行服务器

void test_client(); // 模拟客户端登录

int main() 
{
    std::cout << "网关模块" << std::endl;
    // 启动服务器
    RunServer(); // 运行服务器

    return 0; // 返回0表示程序正常结束
}

void RunServer()
{
    GatewayServerImpl gateway_server; // 网关服务器实现

    grpc::ServerBuilder builder; // gRPC服务器构建器
    std::string server_address("0.0.0.0:50051"); // 网关服务器监听50051端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&gateway_server); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // 构建并启动服务器
    std::cout << "GatewayServer start..." << std::endl; // 输出服务器运行信息

	gateway_server.register_server(); // 注册服务器

    test_client();  // 模拟客户端登录

    server->Wait(); // 等待服务器终止
	gateway_server.unregister_server(); // 注销服务器
}

void test_client()
{
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()); // 链接网关服务器
    auto stub = myproject::GatewayServer::NewStub(channel); // 服务存根

	// 模拟登录数据
	std::string user_name = "lhw";
	std::string password = "159357";

    // 构造登录请求
    myproject::LoginRequest login_request;
    login_request.set_username(user_name); // 设置用户名
    login_request.set_password(password); // 设置密码

	// 包装为转发请求
	myproject::ForwardRequest forward_request;
	forward_request.set_service_type(myproject::ServiceType::REQ_LOGIN); // 设置服务类型
	login_request.SerializeToString(forward_request.mutable_payload()); // 序列化登录请求

    // 构造响应
    myproject::ForwardResponse forward_response;
    grpc::ClientContext client_context; // 包含 RPC 调用的元数据和其他信息

    // 向网关服务器发送请求
    grpc::Status status = stub->RequestForward(&client_context, forward_request, &forward_response);

    if (status.ok() && forward_response.success())
    {
        std::cout << "登录成功" << std::endl;
    }
    else
    {
        std::cout << "登录失败" << std::endl;
    }
}