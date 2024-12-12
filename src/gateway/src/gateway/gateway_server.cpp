#include "gateway_server.h"

#include "server_login.grpc.pb.h"
#include "server_logic.grpc.pb.h"
#include "server_central.grpc.pb.h"

// GatewayServerImpl ���캯��
GatewayServerImpl::GatewayServerImpl() 
{
    // ���ķ�����
    central_channel = grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials());
    central_stub = myproject::CentralServer::NewStub(central_channel);
	// ��¼������
	login_channel = grpc::CreateChannel("localhost:50053", grpc::InsecureChannelCredentials());
	login_stub = myproject::LoginServer::NewStub(login_channel);
}

grpc::Status GatewayServerImpl::RequestForward(grpc::ServerContext* context, const myproject::ForwardRequest* request, myproject::ForwardResponse* response)
{
	switch (request->service_type()) // ��������ķ������ͽ���ת��
    {
    case myproject::ServiceType::LOGIN: // �û���¼
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

    // ���õ�¼���񲢻�ȡ��Ӧ
    myproject::LoginResponse login_response;
    grpc::ClientContext context;
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
