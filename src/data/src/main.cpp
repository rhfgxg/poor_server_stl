#include "./data/data_server.h"

#include <iostream>
#include <grpcpp/grpcpp.h>
#include <mysqlx/xdevapi.h>// mysql
#include <cstdlib> // ʹ�� std::exit

mysqlx::Session sql_link(); // �������ݿ�
void RunServer();    // ���з�����

int main()
{
    mysqlx::Session session = sql_link();   // �������ݿ�

    RunServer();    // ���з�����

    // �ر�����
    session.close();
    return 0;
}


mysqlx::Session sql_link()
{
    try {
        std::cout << "database link ing..." << std::endl;
        mysqlx::SessionSettings option("localhost", 33060, "root", "159357");
        mysqlx::Session session(option);
        std::cout << "detabse link over..." << std::endl;

        // �������ݿ�
        mysqlx::Schema schema = session.getSchema("poor_users");
        std::cout << "poor_users link over..." << std::endl;
        return session;
    }
    catch (const mysqlx::Error& err) {
        std::cerr << "database link Error: " << err.what() << std::endl;
        std::exit(EXIT_FAILURE); // �������У���ʾ���ݿ����
    }
    catch (std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        std::exit(EXIT_FAILURE); // �������У���ʾ���ݿ����
    }
    catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
        std::exit(EXIT_FAILURE); // �������У���ʾ���ݿ����
    }
}

// ���з�����
void RunServer()
{
	std::cout << "����������¼������..." << std::endl; // ���������������Ϣ
    DatabaseServiceImpl service; // ���ݿ�rpc����ʵ��

    grpc::ServerBuilder builder;
    std::string server_address("0.0.0.0:50052");
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "��¼��������������..." << std::endl;
    server->Wait();
}