#ifndef GATEWAY_SERVER_H
#define GATEWAY_SERVER_H

#include "server_gateway.grpc.pb.h"
#include "server_central.grpc.pb.h"
#include "server_login.grpc.pb.h"

#include "connection_pool.h"    // ���ӳ�

#include <grpcpp/grpcpp.h>

// ���ط���������ӿ�
class GatewayServerImpl final : public myproject::GatewayServer::Service
{
public:
	GatewayServerImpl();	// ���캯��

	void register_server(); // ע�������
	void unregister_server(); // ע��������

    // ת����������
    grpc::Status RequestForward(grpc::ServerContext* context, const myproject::ForwardRequest* request, myproject::ForwardResponse* response);

private:
    // ��ʼ�����ӳ�
	void init_connection_pool();

	// ��¼����������¼����
    grpc::Status forward_to_login_service(const std::string& payload, myproject::ForwardResponse* response);

	ConnectionPool login_connection_pool;   // ��¼���������ӳ�
	std::unique_ptr<myproject::CentralServer::Stub> central_stub;	// ���ķ������ķ�����
};

#endif // GATEWAY_SERVER_H