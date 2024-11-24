#ifndef GATEWAY_SERVICE_H
#define GATEWAY_SERVICE_H

#include <grpcpp/grpcpp.h>
#include "server_login.grpc.pb.h"

class GatewayService {
public:
	// 参数是一个指向Channel的智能指针，用来和远程通信
    GatewayService(std::shared_ptr<grpc::Channel> channel);
    std::string Login(const std::string& username, const std::string& password);

private:
    std::unique_ptr<myproject::LoginService::Stub> stub_;
};

#endif // !GATEWAY_SERVICE_H