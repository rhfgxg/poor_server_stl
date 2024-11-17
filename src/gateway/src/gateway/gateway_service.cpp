#include "./gateway_service.h"

// GatewayService ���캯������ʼ�� stub_
GatewayService::GatewayService(std::shared_ptr<grpc::Channel> channel) : stub_(myproject::LoginService::NewStub(channel)) 
{

}

// Login �����������¼����
std::string GatewayService::Login(const std::string& username, const std::string& password) 
{
    myproject::LoginRequest request;    // �������
    request.set_username(username); // �����û���
    request.set_password(password); // ��������

	myproject::LoginResponse response;  // ���ز���
    grpc::ClientContext context;    // ���� RPC ���õ�Ԫ���ݺ�������Ϣ��

    grpc::Status status = stub_->Login(&context, request, &response); // ���¼���������͵�¼����

    if (status.ok()) 
    {
        return response.message(); // ������Ӧ��Ϣ
    }
    else 
    {
        return "RPC failed"; // ���ش�����Ϣ
    }
}
