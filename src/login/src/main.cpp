#include <iostream>
#include <grpcpp/grpcpp.h>
#include "./login/login_server.h"

// ���з�����
void RunServer();

int main() 
{
    RunServer(); // ���з�����
    return 0; // ����0��ʾ������������
}

void RunServer()
{
    LoginServiceImpl service; // ��¼����ʵ��

    grpc::ServerBuilder builder; // gRPC������������
    std::string server_address("0.0.0.0:50051"); // ����������50051�˿�
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // ��Ӽ����˿�
    builder.RegisterService(&service); // ע�����

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // ����������������
    std::cout << "��¼��������������..." << std::endl; // ���������������Ϣ
    server->Wait(); // �ȴ���������ֹ
}