#ifndef GATEWAY_SERVER_H
#define GATEWAY_SERVER_H

#include "server_gateway.grpc.pb.h"
#include "server_central.grpc.pb.h"
#include "server_login.grpc.pb.h"
#include "connection_pool.h"    // 连接池
#include "logger_manager.h"     // 日志管理器

#include <grpcpp/grpcpp.h>

// 网关服务器对外接口
class GatewayServerImpl final : public myproject::GatewayServer::Service
{
public:
	GatewayServerImpl(LoggerManager& logger_manager_);	// 构造函数

	void register_server(); // 注册服务器
	void unregister_server(); // 注销服务器

    // 转发服务请求
    grpc::Status RequestForward(grpc::ServerContext* context, const myproject::ForwardRequest* request, myproject::ForwardResponse* response);

private:
    // 初始化链接池
	void init_connection_pool();

	// 登录服务器：登录服务
    grpc::Status forward_to_login_service(const std::string& payload, myproject::ForwardResponse* response);

    // 日志管理器
    LoggerManager& logger_manager;

	ConnectionPool login_connection_pool;   // 登录服务器连接池
	std::unique_ptr<myproject::CentralServer::Stub> central_stub;	// 中心服务器的服务存根
};

#endif // GATEWAY_SERVER_H
