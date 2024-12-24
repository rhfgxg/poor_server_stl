#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H

// 中心服务器实现类，负责处理中心服务器的所有请求

#include "server_central.grpc.pb.h" // 中心服务器
#include "server_data.grpc.pb.h"    // 数据库服务器
#include "server_gateway.grpc.pb.h" // 网关服务器
#include "server_login.grpc.pb.h"   // 登录服务器
#include "connection_pool.h"    // 连接池
#include "logger_manager.h"     // 日志管理器

#include <grpcpp/grpcpp.h>
#include <thread>
#include <chrono>
 
// 中心服务器实现类
class CentralServerImpl final : public myproject::CentralServer::Service
{
public:
    CentralServerImpl(LoggerManager& logger_manager_);
	~CentralServerImpl();  

    // 服务器注册
    grpc::Status RegisterServer(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response);
    // 服务器断开
    grpc::Status UnregisterServer(grpc::ServerContext* context, const myproject::UnregisterServerRequest* request, myproject::UnregisterServerResponse* response);
    
    // 获取连接池中所有链接
    grpc::Status GetConnectPoor(grpc::ServerContext* context, const myproject::ConnectPoorRequest* request, myproject::ConnectPoorResponse* response);

private:
	// 日志管理器
	LoggerManager& logger_manager;

    // 连接池
	ConnectionPool central_connection_pool; // 多中心服务器，所有中心服务器都维护同一个连接池
    ConnectionPool data_connection_pool;
    ConnectionPool gateway_connection_pool;
    ConnectionPool logic_connection_pool;
    ConnectionPool login_connection_pool;
};


#endif // CENTRAL_SERVER_H
