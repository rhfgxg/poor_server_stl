#ifndef LOGIN_SERVER_H
#define LOGIN_SERVER_H

#include "server_data.grpc.pb.h"    // ���ݿ����
#include "server_login.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <map>
#include <string>

// ��¼����ʵ����
class LoginServerImpl final : public myproject::LoginServer::Service {
public:
	LoginServerImpl();	// ���캯��

    grpc::Status Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) override;  // ��¼
    grpc::Status Register(grpc::ServerContext* context, const myproject::RegisterRequest* request, myproject::RegisterResponse* response) override;   // ע��
    grpc::Status Authenticate(grpc::ServerContext* context, const myproject::AuthenticateRequest* request, myproject::AuthenticateResponse* response) override;   // ������֤

private:
	std::string login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query);    // ��¼
	std::string register_(const std::string& database, const std::string& table, std::map<std::string, std::string> data);    // ע��
	std::string authenticate_(const std::string& token);    // ������֤
	
	std::shared_ptr<grpc::Channel> channel_;	// grpcͨ��
	std::unique_ptr<myproject::DatabaseServer::Stub> stub_;	// ���ݿ������
};

#endif // LOGIN_SERVER_H