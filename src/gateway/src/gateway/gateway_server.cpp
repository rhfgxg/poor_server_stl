#include "gateway_server.h"
#include <thread>
#include <chrono>

// GatewayServerImpl ���캯��
GatewayServerImpl::GatewayServerImpl() :   
    central_stub(myproject::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))), // ���ķ��������
    login_connection_pool(10) // ��ʼ����¼���������ӳأ��������ӳش�СΪ10
{
    // ������ʱ���񣬶�ʱ�����ķ�������ȡ���µ����ӳ�״̬
}

// ע�������
void GatewayServerImpl::register_server()
{
    // ����
    myproject::RegisterServerRequest request;
    request.set_server_type(myproject::ServerType::GATEWAY);
    request.set_address("127.0.0.1");
    request.set_port("50051");
    // ��Ӧ
    myproject::RegisterServerResponse response;

    // �ͻ���
    grpc::ClientContext context;

    grpc::Status status = central_stub->RegisterServer(&context, request, &response);

    if (status.ok() && response.success())
    {
        std::cout << "������ע��ɹ�: " << response.message() << std::endl;
		init_connection_pool(); // ��ʼ�����ӳ�
    }
    else
    {
        std::cerr << "������ע��ʧ��: " << response.message() << std::endl;
    }
}

// ע��������
void GatewayServerImpl::unregister_server()
{
    // �ͻ���
    grpc::ClientContext context;
    // ����
    myproject::UnregisterServerRequest request;
    request.set_server_type(myproject::ServerType::GATEWAY);
    request.set_address("localhost");
    request.set_port("50051");

    // ��Ӧ
    myproject::UnregisterServerResponse response;

    grpc::Status status = central_stub->UnregisterServer(&context, request, &response);

    if (status.ok() && response.success()) {
        std::cout << "������ע���ɹ�: " << response.message() << std::endl;
    }
    else {
        std::cerr << "������ע��ʧ��: " << response.message() << std::endl;
    }
}

// ��ʼ�����ӳ�
void GatewayServerImpl::init_connection_pool()
{
    // �ͻ���
    grpc::ClientContext context;
    // ����
    myproject::ConnectPoorRequest request;
    request.set_server_type(myproject::ServerType::LOGIN);
    // ��Ӧ
    myproject::ConnectPoorResponse response;

    grpc::Status status = central_stub->GetConnectPoor(&context, request, &response);

    if (status.ok())
    {
        // ���µ�¼���������ӳ�
        for (const auto& server_info : response.connect_info())
        {
            login_connection_pool.add_server(myproject::ServerType::LOGIN, server_info.address(), std::to_string(server_info.port()));
        }
    }
    else
    {
        std::cerr << "�޷���ȡ��¼���������ӳ���Ϣ: " << status.error_message() << std::endl;
    }
}

// ����ת���ӿ�
grpc::Status GatewayServerImpl::RequestForward(grpc::ServerContext* context, const myproject::ForwardRequest* request, myproject::ForwardResponse* response)
{

    switch (request->service_type()) // ��������ķ������ͽ���ת��
    {
    case myproject::ServiceType::REQ_LOGIN: // �û���¼����
    {
        return forward_to_login_service(request->payload(), response);  
    }
    default:    // δ֪��������
        response->set_success(false);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Unknown service type");
    }
}

// Login �����������¼����
grpc::Status GatewayServerImpl::forward_to_login_service(const std::string& payload, myproject::ForwardResponse* response)
{
    myproject::LoginRequest login_request;  // ������¼�������
    bool request_out = login_request.ParseFromString(payload); // �����ؽ���Ϊ��¼�������
    if (!request_out) // �������ʧ��
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Failed to parse LoginRequest");
    }

    // ������Ӧ
    myproject::LoginResponse login_response;
    grpc::ClientContext context;

    // ��ȡ���ӳ��е�����
    auto channel = login_connection_pool.get_connection(myproject::ServerType::LOGIN);
    auto login_stub = myproject::LoginServer::NewStub(channel);

    grpc::Status status = login_stub->Login(&context, login_request, &login_response);

    if (!status.ok()) // �������ʧ��
    {
        return status;
    }

    bool response_out = login_response.SerializeToString(response->mutable_response()); // ����¼��Ӧ���л�Ϊת����Ӧ
    if (!response_out) // ������л�ʧ��
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to serialize LoginResponse");
    }

    response->set_success(true);
    return grpc::Status::OK;
}
