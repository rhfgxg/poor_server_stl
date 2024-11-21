#ifndef LOGIN_SERVICE_H
#define LOGIN_SERVICE_H
#include <grpcpp/grpcpp.h>
#include "../../../protobuf/login/server_login.grpc.pb.h"

class LoginServiceImpl final : public myproject::LoginService::Service {
public:
    grpc::Status Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) override;
};

#endif