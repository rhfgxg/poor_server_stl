#include "./data/data_server.h"

#include <iostream>
#include <grpcpp/grpcpp.h>
#include <mysqlx/xdevapi.h> // mysql
#include <cstdlib> // 使用 std::exit

mysqlx::Session sql_link(); // 链接数据库
void RunServer(mysqlx::Session& sql_link); // 运行服务器

int main()
{
	// 启动服务器
    mysqlx::Session session = sql_link(); // 链接数据库
    RunServer(session); // 运行服务器

    // 关闭服务器
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
	service.register_server();

    server->Wait();
    // 注销服务器
	service.unregister_server();
}
