#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H

#include "server_central.grpc.pb.h" // 中心服务器
#include "server_data.grpc.pb.h"    // 数据库服务器
#include "server_gateway.grpc.pb.h" // 网关服务器
#include "server_login.grpc.pb.h"   // 登录服务器
#include "connection_pool.h"    // 连接池

#include <grpcpp/grpcpp.h>
#include <cpp_redis/cpp_redis>
#include <thread>
#include <chrono>

// 暂时不使用 redis保存服务器信息
//#include <WinSock2.h>
//#include <WS2tcpip.h>
//// 工具类，封装redis链接，防止 windows 下的一个异常
//class RedisClient
//{
///* redis 与 WINsock 冲突问题
// * redis 默认全局初始化了 WINsock，导致 grpc 无法正常工作
// * 使用该类封装 redis 客户端，避免 redis 初始化 WINsock
// * 由于 redis 客户端是异步的，所以使用 shared_ptr 管理 redis 客户端
// * 由于这是windows下的问题，所以只在windows下使用该类
// */
//public:
//    RedisClient()
//    {
//    #ifdef _WIN32   // 下面代码使用了 Windows API：WSAStartup
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
   
// 中心服务器对外接口
class CentralServerImpl final : public myproject::CentralServer::Service
{
public:
    explicit CentralServerImpl();
	~CentralServerImpl();  

    // 服务器注册
    grpc::Status RegisterServer(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response);
    // 服务器断开
    grpc::Status UnregisterServer(grpc::ServerContext* context, const myproject::UnregisterServerRequest* request, myproject::UnregisterServerResponse* response);
    
    // 获取连接池中所有链接
    grpc::Status GetConnectPoor(grpc::ServerContext* context, const myproject::ConnectPoorRequest* request, myproject::ConnectPoorResponse* response);

private:
    //RedisClient redis_client; // Redis 客户端

    // 连接池
	ConnectionPool central_connection_pool; // 考虑到多中心服务器，每个中心服务器都有一个连接池
    ConnectionPool data_connection_pool;
    ConnectionPool gateway_connection_pool;
    ConnectionPool login_connection_pool;
};


#endif // CENTRAL_SERVER_H