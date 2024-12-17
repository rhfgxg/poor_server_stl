#include "login/login_server.h"

#include <iostream>

void RunServer();   // ���з�����

int main() 
{
    // ����������
    RunServer(); // ���з�����

    // �رշ�����
    return 0; // ����0��ʾ������������
}

void RunServer()
{
    LoginServerImpl login_server; // ��¼����ʵ��

    grpc::ServerBuilder builder; // gRPC������������
    std::string server_address("0.0.0.0:50053"); // ��¼����������50053�˿�
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // ��Ӽ����˿�
    builder.RegisterService(&login_server); // ע�����

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // ����������������
    std::cout << "LoginServer start..." << std::endl; // ���������������Ϣ

    // ע�������
	login_server.register_server(); // ע�������

    server->Wait(); // �ȴ���������ֹ
	// ע��������
	login_server.unregister_server(); // ע��������
}
