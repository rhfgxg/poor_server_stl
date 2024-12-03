#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H


#include "server_central.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <mysqlx/xdevapi.h> // mysql

// 中心服务器对外接口
class CentralServerImpl final : public myproject::CentralServer::Service 
{
public:
    // 服务器注册
    grpc::Status register_server(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response) override;   
    // 获取目标服务器信息
    grpc::Status get_server_info(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response) override;   

};


#endif // CENTRAL_SERVER_H