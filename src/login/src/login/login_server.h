#ifndef LOGIN_SERVER_H
#define LOGIN_SERVER_H

#include "server_login.grpc.pb.h"	// ��¼����
#include "server_data.grpc.pb.h"    // ���ݿ����
#include "server_central.grpc.pb.h"	// ���ķ���

#include "connection_pool.h"    // ���ӳ�

#include <grpcpp/grpcpp.h>
#include <map>
#include <string>

// ��¼����ʵ����
class LoginServerImpl final : public myproject::LoginServer::Service {
public:
	LoginServerImpl();	// ���캯��

	void register_server(); // ע�������
	void unregister_server(); // ע��������

    grpc::Status Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) override;  // ��¼
    grpc::Status Register(grpc::ServerContext* context, const myproject::RegisterRequest* request, myproject::RegisterResponse* response) override;   // ע��
    grpc::Status Authenticate(grpc::ServerContext* context, const myproject::AuthenticateRequest* request, myproject::AuthenticateResponse* response) override;   // ������֤

private:
	// ��ʼ�����ӳ�
	void init_connection_pool();

	std::string login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query);    // ��¼
	std::string register_(const std::string& database, const std::string& table, std::map<std::string, std::string> data);    // ע��
	std::string authenticate_(const std::string& token);    // ������֤
	
	std::unique_ptr<myproject::CentralServer::Stub> central_stub;	// ���ķ�����
	ConnectionPool db_connection_pool;   // ��¼���������ӳ�
};

#endif // LOGIN_SERVER_H