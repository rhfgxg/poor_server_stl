#include "./gateway/gateway_server.h"
#include "../../central/src/central/central_server.h"
#include "../../login/src/login/login_server.h"

#include <iostream>
#include <grpcpp/grpcpp.h>

void RunServer();   // ���з�����
void RegisterServer(); // ע�������
void UnregisterServer(); // ע��������

void test_client(); // ģ��ͻ��˵�¼

int main() 
{
    std::cout << "����ģ��" << std::endl;
    // ����������
    RunServer(); // ���з�����

    /*
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1234));

    std::cout << "�ȴ��ͻ�������..." << std::endl;

    while (1)
    {
        boost::asio::ip::tcp::socket socket(io_context);
        acceptor.accept(socket);

        std::cout << "�ͻ������ӳɹ�" << std::endl;

        // ��ȡ�ͻ������󣨼����������û��������룩
        boost::asio::streambuf buffer;
        boost::asio::read_until(socket, buffer, "\n");
        std::istream is(&buffer);
        std::string username, password;
        is >> username >> password;

        // ���õ�¼������
        GatewayService gateway_service(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
        std::string response = gateway_service.Login(username, password);

        // ������Ӧ���ͻ���
        boost::asio::write(socket, boost::asio::buffer(response + "\n"));
    */

    //// ģ���¼����
    //std::string user_name = "lhw";
    //std::string password = "159357";

    // �رշ�����
    UnregisterServer(); // ע��������
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

    // ע�������
    RegisterServer();

    test_client();  // ģ��ͻ��˵�¼

    server->Wait(); // �ȴ���������ֹ
}

void RegisterServer()
{
    auto channel = grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()); // �������ķ�����
    auto stub = myproject::CentralServer::NewStub(channel); // ������

    // ����
    myproject::RegisterServerRequest request;
    request.set_server_name("GatewayServer");
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
    request.set_server_name("GatewayServer");

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
	forward_request.set_service_type(myproject::ServiceType::LOGIN); // ���÷�������
	login_request.SerializeToString(forward_request.mutable_payload()); // ���л���¼����

    // ������Ӧ
    myproject::ForwardResponse forward_response;
    grpc::ClientContext client_context; // ���� RPC ���õ�Ԫ���ݺ�������Ϣ

    // �����ݿ���������Ͳ�ѯ����
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