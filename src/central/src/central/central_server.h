#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H

#include "server_central.grpc.pb.h" // ���ķ�����
#include "server_data.grpc.pb.h"    // ���ݿ������
#include "server_gateway.grpc.pb.h" // ���ط�����
#include "server_login.grpc.pb.h"   // ��¼������
#include "connection_pool.h"    // ���ӳ�

#include <grpcpp/grpcpp.h>
#include <cpp_redis/cpp_redis>
#include <thread>
#include <chrono>

// ��ʱ��ʹ�� redis�����������Ϣ
//#include <WinSock2.h>
//#include <WS2tcpip.h>
//// �����࣬��װredis���ӣ���ֹ windows �µ�һ���쳣
//class RedisClient
//{
///* redis �� WINsock ��ͻ����
// * redis Ĭ��ȫ�ֳ�ʼ���� WINsock������ grpc �޷���������
// * ʹ�ø����װ redis �ͻ��ˣ����� redis ��ʼ�� WINsock
// * ���� redis �ͻ������첽�ģ�����ʹ�� shared_ptr ���� redis �ͻ���
// * ��������windows�µ����⣬����ֻ��windows��ʹ�ø���
// */
//public:
//    RedisClient()
//    {
//    #ifdef _WIN32   // �������ʹ���� Windows API��WSAStartup
//        WORD version = MAKEWORD(2, 2);
//        WSADATA data;
//        if (WSAStartup(version, &data) != 0) {
//            throw std::runtime_error("WSAStartup failed");
//        }
//    #endif
//        m_client = std::make_shared<cpp_redis::client>();
//    }
//
//    ~RedisClient()
//    {
//#ifdef _WIN32
//        WSACleanup();
//#endif
//    }
//
//    std::shared_ptr<cpp_redis::client> get_client() const {
//        return m_client;
//    }
//
//private:
//    std::shared_ptr<cpp_redis::client> m_client;
//};
   
// ���ķ���������ӿ�
class CentralServerImpl final : public myproject::CentralServer::Service
{
public:
    explicit CentralServerImpl();
	~CentralServerImpl();  

    // ������ע��
    grpc::Status RegisterServer(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response);
    // �������Ͽ�
    grpc::Status UnregisterServer(grpc::ServerContext* context, const myproject::UnregisterServerRequest* request, myproject::UnregisterServerResponse* response);
    
    // ��ȡ���ӳ�����������
    grpc::Status GetConnectPoor(grpc::ServerContext* context, const myproject::ConnectPoorRequest* request, myproject::ConnectPoorResponse* response);

private:
    //RedisClient redis_client; // Redis �ͻ���

    // ���ӳ�
	ConnectionPool central_connection_pool; // ���ǵ������ķ�������ÿ�����ķ���������һ�����ӳ�
    ConnectionPool data_connection_pool;
    ConnectionPool gateway_connection_pool;
    ConnectionPool login_connection_pool;
};


#endif // CENTRAL_SERVER_H