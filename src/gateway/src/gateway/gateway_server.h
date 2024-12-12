#ifndef GATEWAY_SERVER_H
#define GATEWAY_SERVER_H

#include "server_gateway.grpc.pb.h"
#include "server_login.grpc.pb.h"
#include "server_central.grpc.pb.h"
#include <grpcpp/grpcpp.h>

// 网关服务器对外接口
class GatewayServerImpl final : public myproject::GatewayServer::Service
{
public:
	GatewayServerImpl();	// 构造函数

    // 转发服务请求
    grpc::Status RequestForward(grpc::ServerContext* context, const myproject::ForwardRequest* request, myproject::ForwardResponse* response);

private:
	// 登录服务器：登录服务
    grpc::Status forward_to_login_service(const std::string& payload, myproject::ForwardResponse* response);

	// 登录服务器
	std::shared_ptr<grpc::Channel> login_channel;
    std::unique_ptr<myproject::LoginServer::Stub> login_stub;
    // 逻辑服
    //std::shared_ptr<grpc::Channel> logic_channel;
    //std::unique_ptr<myproject::LogicServer::Stub> logic_stub;
	// 中心服务器
    std::shared_ptr<grpc::Channel> central_channel;
    std::unique_ptr<myproject::CentralServer::Stub> central_stub;
};

#endif // GATEWAY_SERVER_H