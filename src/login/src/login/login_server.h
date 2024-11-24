#ifndef LOGIN_SERVER_H
#define LOGIN_SERVER_H

#include "server_login.grpc.pb.h"
#include <grpcpp/grpcpp.h>

// ��¼����ʵ����
class LoginServiceImpl final : public myproject::LoginService::Service {
public:
    grpc::Status Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) override;
};

#endif // LOGIN_SERVER_H