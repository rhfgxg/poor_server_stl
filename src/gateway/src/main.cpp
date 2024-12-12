#include "./gateway/gateway_server.h"
#include "../../central/src/central/central_server.h"
#include "../../login/src/login/login_server.h"

#include <iostream>
#include <grpcpp/grpcpp.h>

void RunServer();   // 运行服务器
void RegisterServer(); // 注册服务器
void UnregisterServer(); // 注销服务器

void test_client(); // 模拟客户端登录

int main() 
{
    std::cout << "网关模块" << std::endl;
    // 启动服务器
    RunServer(); // 运行服务器

    /*
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1234));

    std::cout << "等待客户端连接..." << std::endl;

    while (1)
    {
        boost::asio::ip::tcp::socket socket(io_context);
        acceptor.accept(socket);

        std::cout << "客户端连接成功" << std::endl;

        // 读取客户端请求（假设请求是用户名和密码）
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, "\n");
        std::istream is(&buffer);
        std::string username, password;
        is >> username >> password;

        // 调用登录服务器
        GatewayService gateway_service(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
        std::string response = gateway_service.Login(username, password);

        // 发送响应给客户端
        boost::asio::write(socket, boost::asio::buffer(response + "\n"));
    */

    //// 模拟登录数据
    //std::string user_name = "lhw";
    //std::string password = "159357";

    // 关闭服务器
    UnregisterServer(); // 注销服务器
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

    // 注册服务器
    RegisterServer();

    test_client();  // 模拟客户端登录

    server->Wait(); // 等待服务器终止
}

void RegisterServer()
{
    auto channel = grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()); // 链接中心服务器
    auto stub = myproject::CentralServer::NewStub(channel); // 服务存根

    // 请求
    myproject::RegisterServerRequest request;
    request.set_server_name("GatewayServer");
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
    request.set_server_name("GatewayServer");

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
	forward_request.set_service_type(myproject::ServiceType::LOGIN); // 设置服务类型
	login_request.SerializeToString(forward_request.mutable_payload()); // 序列化登录请求

    // 构造响应
    myproject::ForwardResponse forward_response;
    grpc::ClientContext client_context; // 包含 RPC 调用的元数据和其他信息

    // 向数据库服务器发送查询请求
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