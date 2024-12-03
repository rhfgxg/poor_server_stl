#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H


#include "server_central.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <mysqlx/xdevapi.h> // mysql

// ���ķ���������ӿ�
class CentralServerImpl final : public myproject::CentralServer::Service 
{
public:
    // ������ע��
    grpc::Status register_server(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response) override;   
    // ��ȡĿ���������Ϣ
    grpc::Status get_server_info(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response) override;   

};


#endif // CENTRAL_SERVER_H