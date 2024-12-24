#include "./data/data_server.h"
#include "logger_manager.h" // 引入日志管理器
#include <iostream>
#include <mysqlx/xdevapi.h> // mysql
#include <cstdlib> // 使用 std::exit

// 数据库服务器main函数

mysqlx::Session sql_link(); // 链接数据库
void RunServer(LoggerManager& logger_manager, mysqlx::Session& sql_link); // 运行服务器

int main()
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(myproject::ServerType::DATA);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("数据库服务器启动"); // 记录启动日志：日志分类, 日志内容

    // 启动服务器
    mysqlx::Session session = sql_link(); // 链接数据库

    RunServer(logger_manager, session); // 运行服务器

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

void RunServer(LoggerManager& logger_manager,mysqlx::Session& sql_link)
{
    DatabaseServerImpl service(logger_manager, sql_link); // 数据库rpc服务实现，传入数据库链接

    grpc::ServerBuilder builder;
    std::string server_address("0.0.0.0:50052");    // 数据库服务器监听50052端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("DataServer 启动，监听地址: {}",server_address);

    // 注册服务器
    service.register_server();

    server->Wait();
    // 注销服务器
    service.unregister_server();
}
