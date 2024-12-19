#ifndef LOGIN_SERVER_H
#define LOGIN_SERVER_H

#include "server_login.grpc.pb.h"	// 登录服务
#include "server_data.grpc.pb.h"    // 数据库服务
#include "server_central.grpc.pb.h"	// 中心服务

#include "connection_pool.h"    // 连接池

#include <grpcpp/grpcpp.h>
#include <map>
#include <string>

// 登录服务实现类
class LoginServerImpl final : public myproject::LoginServer::Service {
public:
	LoginServerImpl();	// 构造函数

	void register_server(); // 注册服务器
	void unregister_server(); // 注销服务器

    grpc::Status Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) override;  // 登录
    grpc::Status Register(grpc::ServerContext* context, const myproject::RegisterRequest* request, myproject::RegisterResponse* response) override;   // 注册
    grpc::Status Authenticate(grpc::ServerContext* context, const myproject::AuthenticateRequest* request, myproject::AuthenticateResponse* response) override;   // 令牌验证

private:
	// 初始化链接池
	void init_connection_pool();

	std::string login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query);    // 登录
	std::string register_(const std::string& database, const std::string& table, std::map<std::string, std::string> data);    // 注册
	std::string authenticate_(const std::string& token);    // 令牌验证
	
	std::unique_ptr<myproject::CentralServer::Stub> central_stub;	// 中心服务存根
	ConnectionPool db_connection_pool;   // 登录服务器连接池
};

#endif // LOGIN_SERVER_H