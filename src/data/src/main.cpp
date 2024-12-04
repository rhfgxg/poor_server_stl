#include "./data/data_server.h"
#include "../../central/src/central/central_server.h"

#include <iostream>
#include <grpcpp/grpcpp.h>
#include <mysqlx/xdevapi.h> // mysql
#include <cstdlib> // ʹ�� std::exit

mysqlx::Session sql_link(); // �������ݿ�
void RunServer(mysqlx::Session& sql_link); // ���з�����
void RegisterServer(); // ע�������
void UnregisterServer(); // ע��������

int main()
{
	// ����������
    mysqlx::Session session = sql_link(); // �������ݿ�
    RunServer(session); // ���з�����

    // �رշ�����
	UnregisterServer(); // ע��������
    session.close();    // �ر����ݿ�����
    return 0;
}

mysqlx::Session sql_link()
{
    try {
        std::cout << "database link ing..." << std::endl;
        mysqlx::SessionSettings option("localhost", 33060, "root", "159357");
        mysqlx::Session session(option);
        std::cout << "database link over..." << std::endl;

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

void RunServer(mysqlx::Session& sql_link)
{
    DatabaseServerImpl service(sql_link); // ���ݿ�rpc����ʵ�֣��������ݿ�����

    grpc::ServerBuilder builder;
	std::string server_address("0.0.0.0:50052");    // ���ݿ����������50052�˿�
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "DataServer start..." << std::endl; // ���������������Ϣ

    // ע�������
    RegisterServer();

    server->Wait();
}

void RegisterServer()
{
    auto channel = grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()); // �������ķ�����
    auto stub = myproject::CentralServer::NewStub(channel); // ������

    // ����
    myproject::RegisterServerRequest request;
    request.set_server_name("DataServer");
    request.set_address("127.0.0.1");
    request.set_port(50052);

    // ��Ӧ
    myproject::RegisterServerResponse response;

    // �ͻ���
    grpc::ClientContext context;

    grpc::Status status = stub->RegisterServer(&context, request, &response);

    if (status.ok() && response.success()) {
        std::cout << "������ע��ɹ�: " << response.message() << std::endl;
    }
    else {
        std::cerr << "������ע��ʧ��: " << response.message() << std::endl;
    }
}

void UnregisterServer()
{
    auto channel = grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()); // �������ķ�����
    auto stub = myproject::CentralServer::NewStub(channel); // ������

    // ����
    myproject::UnregisterServerRequest request;
    request.set_server_name("DataServer");

    // ��Ӧ
    myproject::UnregisterServerResponse response;

    // �ͻ���
    grpc::ClientContext context;

    grpc::Status status = stub->UnregisterServer(&context, request, &response);

    if (status.ok() && response.success()) {
        std::cout << "������ע���ɹ�: " << response.message() << std::endl;
    }
    else {
        std::cerr << "������ע��ʧ��: " << response.message() << std::endl;
    }
}