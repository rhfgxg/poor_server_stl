#ifndef LOGIN_SERVER_H
#define LOGIN_SERVER_H

#include "server_login.grpc.pb.h"
#include <grpcpp/grpcpp.h>

// 登录服务实现类
class LoginServiceImpl final : public myproject::LoginService::Service {
public:
    grpc::Status Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) override;  // 登录
    grpc::Status Register(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) override;   // 注册
    grpc::Status Authenticate(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) override;   // 令牌验证
};

#endif // LOGIN_SERVER_H