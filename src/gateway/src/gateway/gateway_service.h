#ifndef GATEWAY_SERVICE_H
#define GATEWAY_SERVICE_H

#include <grpcpp/grpcpp.h>
#include "server_login.grpc.pb.h"

class GatewayService {
public:
	// ������һ��ָ��Channel������ָ�룬������Զ��ͨ��
    GatewayService(std::shared_ptr<grpc::Channel> channel);
    std::string Login(const std::string& username, const std::string& password);

private:
    std::unique_ptr<myproject::LoginService::Stub> stub_;
};

#endif // !GATEWAY_SERVICE_H