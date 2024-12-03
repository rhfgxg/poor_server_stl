#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H

#include "server_central.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <cpp_redis/cpp_redis>

class RedisClient
{
public:
    RedisClient()
    {
        WORD version = MAKEWORD(2, 2);
        WSADATA data;
        if (WSAStartup(version, &data) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }

        m_client = std::make_shared<cpp_redis::client>();
    }

    ~RedisClient()
    {
        WSACleanup();
    }

    std::shared_ptr<cpp_redis::client> get_client() const {
        return m_client;
    }

private:
    std::shared_ptr<cpp_redis::client> m_client;
};

// ���ķ���������ӿ�
class CentralServerImpl final : public myproject::CentralServer::Service
{
public:
    explicit CentralServerImpl();
	~CentralServerImpl() override;  

    // ������ע��
    grpc::Status register_server(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response);
    // ��ȡĿ���������Ϣ
    grpc::Status get_server_info(grpc::ServerContext* context, const myproject::ServerInfoRequest* request, myproject::ServerInfoResponse* response);

private:
    RedisClient redis_client; // Redis �ͻ���
};


#endif // CENTRAL_SERVER_H