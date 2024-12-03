#include <iostream>
#include "central/central_server.h"

// ���з�����
void RunServer();

int main()
{
	str::cout << "��ģ��";
    RunServer(); // ���з�����

	return 0;
}

void RunServer()
{
    CentralServerImpl central_server; // ��¼����ʵ��

    grpc::ServerBuilder builder; // gRPC������������
    std::string server_address("0.0.0.0:50050"); // ����������50050�˿�
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // ��Ӽ����˿�
    builder.RegisterService(&central_server); // ע�����

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // ����������������
    std::cout << "LoginServer start..." << std::endl; // ���������������Ϣ
    server->Wait(); // �ȴ���������ֹ
}