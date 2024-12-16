#include "./gateway/gateway_server.h"

#include <iostream>
#include <grpcpp/grpcpp.h>

void RunServer();   // ���з�����

void test_client(); // ģ��ͻ��˵�¼

int main() 
{
    std::cout << "����ģ��" << std::endl;
    // ����������
    RunServer(); // ���з�����

    return 0; // ����0��ʾ������������
}

void RunServer()
{
    GatewayServerImpl gateway_server; // ���ط�����ʵ��

    grpc::ServerBuilder builder; // gRPC������������
    std::string server_address("0.0.0.0:50051"); // ���ط���������50051�˿�
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // ��Ӽ����˿�
    builder.RegisterService(&gateway_server); // ע�����

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // ����������������
    std::cout << "GatewayServer start..." << std::endl; // ���������������Ϣ

	gateway_server.register_server(); // ע�������

    test_client();  // ģ��ͻ��˵�¼

    server->Wait(); // �ȴ���������ֹ
	gateway_server.unregister_server(); // ע��������
}

void test_client()
{
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()); // �������ط�����
    auto stub = myproject::GatewayServer::NewStub(channel); // ������

	// ģ���¼����
	std::string user_name = "lhw";
	std::string password = "159357";

    // �����¼����
    myproject::LoginRequest login_request;
    login_request.set_username(user_name); // �����û���
    login_request.set_password(password); // ��������

	// ��װΪת������
	myproject::ForwardRequest forward_request;
	forward_request.set_service_type(myproject::ServiceType::REQ_LOGIN); // ���÷�������
	login_request.SerializeToString(forward_request.mutable_payload()); // ���л���¼����

    // ������Ӧ
    myproject::ForwardResponse forward_response;
    grpc::ClientContext client_context; // ���� RPC ���õ�Ԫ���ݺ�������Ϣ

    // �����ط�������������
    grpc::Status status = stub->RequestForward(&client_context, forward_request, &forward_response);

    if (status.ok() && forward_response.success())
    {
        std::cout << "��¼�ɹ�" << std::endl;
    }
    else
    {
        std::cout << "��¼ʧ��" << std::endl;
    }
}