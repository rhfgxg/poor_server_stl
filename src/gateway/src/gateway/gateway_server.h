#ifndef GATEWAY_SERVER_H
#define GATEWAY_SERVER_H

#include "server_gateway.grpc.pb.h"
#include "server_login.grpc.pb.h"
#include "server_central.grpc.pb.h"
#include <grpcpp/grpcpp.h>

// ���ط���������ӿ�
class GatewayServerImpl final : public myproject::GatewayServer::Service
{
public:
	GatewayServerImpl();	// ���캯��

    // ת����������
    grpc::Status RequestForward(grpc::ServerContext* context, const myproject::ForwardRequest* request, myproject::ForwardResponse* response);

private:
	// ��¼����������¼����
    grpc::Status forward_to_login_service(const std::string& payload, myproject::ForwardResponse* response);

	// ��¼������
	std::shared_ptr<grpc::Channel> login_channel;
    std::unique_ptr<myproject::LoginServer::Stub> login_stub;
    // �߼���
    //std::shared_ptr<grpc::Channel> logic_channel;
    //std::unique_ptr<myproject::LogicServer::Stub> logic_stub;
	// ���ķ�����
    std::shared_ptr<grpc::Channel> central_channel;
    std::unique_ptr<myproject::CentralServer::Stub> central_stub;
};

#endif // GATEWAY_SERVER_H