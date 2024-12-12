#include "login/login_server.h"
#include "server_login.grpc.pb.h"	// ��¼����

#include "../../central/src/central/central_server.h"

#include <iostream>
#include <grpcpp/grpcpp.h>

void RunServer();   // ���з�����
void RegisterServer(); // ע�������
void UnregisterServer(); // ע��������

int main() 
{
    // ����������
    RunServer(); // ���з�����

    // �رշ�����
    UnregisterServer(); // ע��������
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
    RegisterServer();

    server->Wait(); // �ȴ���������ֹ
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