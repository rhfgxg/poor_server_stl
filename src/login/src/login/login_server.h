#ifndef LOGIN_SERVER_H
#define LOGIN_SERVER_H

#include "server_login.grpc.pb.h"
#include <grpcpp/grpcpp.h>

// 登录服务实现类
class LoginServiceImpl final : public myproject::LoginService::Service {
public:
    grpc::Status Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) override;
};

#endif // LOGIN_SERVER_H