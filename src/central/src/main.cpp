#include "central/central_server.h"
#include <iostream>

// ���з�����
void RunServer();

int main()
{
	std::cout << "��ģ��\n";
    RunServer(); // ���з�����

	return 0;
}

void RunServer()
{
    CentralServerImpl central_server;

    grpc::ServerBuilder builder;
	std::string server_address("0.0.0.0:50050");// ���ķ���������50050�˿�
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // ��Ӽ����˿�
    builder.RegisterService(&central_server); // ע�����

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "CentealServer start..." << std::endl;
    server->Wait();
}