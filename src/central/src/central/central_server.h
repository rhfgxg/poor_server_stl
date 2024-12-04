#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H

#include "server_central.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <cpp_redis/cpp_redis>

// 工具类，封装redis链接，防止引起的 WINsock 错误
class RedisClient
{
/* redis 与 WINsock 冲突问题
 * redis 默认全局初始化了 WINsock，导致 grpc 无法正常工作
 * 使用该类封装 redis 客户端，避免 redis 初始化 WINsock
 * 由于 redis 客户端是异步的，所以使用 shared_ptr 管理 redis 客户端
 * 由于这是windows下的问题，所以只在windows下使用该类
 */
public:
    RedisClient()
    {
#ifdef _WIN32   // 下面代码使用了 Windows API：WSAStartup
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

// 中心服务器对外接口
class CentralServerImpl final : public myproject::CentralServer::Service
{
public:
    explicit CentralServerImpl();
	~CentralServerImpl() override;  

    // 服务器注册
    grpc::Status RegisterServer(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response);
    // 服务器断开
    grpc::Status UnregisterServer(grpc::ServerContext* context, const myproject::UnregisterServerRequest* request, myproject::UnregisterServerResponse* response);
    // 获取目标服务器信息
    grpc::Status GetServerInfo(grpc::ServerContext* context, const myproject::ServerInfoRequest* request, myproject::ServerInfoResponse* response);

private:
    RedisClient redis_client; // Redis 客户端
};


#endif // CENTRAL_SERVER_H