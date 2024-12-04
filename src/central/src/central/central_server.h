#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H

#include "server_central.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <cpp_redis/cpp_redis>

// �����࣬��װredis���ӣ���ֹ����� WINsock ����
class RedisClient
{
/* redis �� WINsock ��ͻ����
 * redis Ĭ��ȫ�ֳ�ʼ���� WINsock������ grpc �޷���������
 * ʹ�ø����װ redis �ͻ��ˣ����� redis ��ʼ�� WINsock
 * ���� redis �ͻ������첽�ģ�����ʹ�� shared_ptr ���� redis �ͻ���
 * ��������windows�µ����⣬����ֻ��windows��ʹ�ø���
 */
public:
    RedisClient()
    {
#ifdef _WIN32   // �������ʹ���� Windows API��WSAStartup
        WORD version = MAKEWORD(2, 2);
        WSADATA data;
        if (WSAStartup(version, &data) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
#endif
        m_client = std::make_shared<cpp_redis::client>();
    }

    ~RedisClient()
    {
#ifdef _WIN32
        WSACleanup();
#endif
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
    grpc::Status RegisterServer(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response);
    // �������Ͽ�
    grpc::Status UnregisterServer(grpc::ServerContext* context, const myproject::UnregisterServerRequest* request, myproject::UnregisterServerResponse* response);
    // ��ȡĿ���������Ϣ
    grpc::Status GetServerInfo(grpc::ServerContext* context, const myproject::ServerInfoRequest* request, myproject::ServerInfoResponse* response);

private:
    RedisClient redis_client; // Redis �ͻ���
};


#endif // CENTRAL_SERVER_H