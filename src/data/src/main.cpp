#include "./data/data_server.h"
#include "../../central/src/central/central_server.h"

#include <iostream>
#include <grpcpp/grpcpp.h>
#include <mysqlx/xdevapi.h> // mysql
#include <cstdlib> // 使用 std::exit

mysqlx::Session sql_link(); // 链接数据库
void RunServer(mysqlx::Session& sql_link); // 运行服务器
void RegisterServer(); // 注册服务器
void UnregisterServer(); // 注销服务器

int main()
{
	// 启动服务器
    mysqlx::Session session = sql_link(); // 链接数据库
    RunServer(session); // 运行服务器

    // 关闭服务器
	UnregisterServer(); // 注销服务器
    session.close();    // 关闭数据库连接
    return 0;
}

mysqlx::Session sql_link()
{
    try {
        std::cout << "database link ing..." << std::endl;
        mysqlx::SessionSettings option("localhost", 33060, "root", "159357");
        mysqlx::Session session(option);
        std::cout << "database link over..." << std::endl;

        // 链接数据库
        mysqlx::Schema schema = session.getSchema("poor_users");
        std::cout << "poor_users link over..." << std::endl;
        return session;
    }
    catch (const mysqlx::Error& err) {
        std::cerr << "database link Error: " << err.what() << std::endl;
        std::exit(EXIT_FAILURE); // 结束运行，提示数据库错误
    }
    catch (std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        std::exit(EXIT_FAILURE); // 结束运行，提示数据库错误
    }
    catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
        std::exit(EXIT_FAILURE); // 结束运行，提示数据库错误
    }
}

void RunServer(mysqlx::Session& sql_link)
{
    DatabaseServerImpl service(sql_link); // 数据库rpc服务实现，传入数据库链接

    grpc::ServerBuilder builder;
	std::string server_address("0.0.0.0:50052");    // 数据库服务器监听50052端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "DataServer start..." << std::endl; // 输出服务器启动信息

    // 注册服务器
    RegisterServer();

    server->Wait();
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