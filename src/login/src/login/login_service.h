#ifndef LOGIN_SERVICE_H
#define LOGIN_SERVICE_H

#include "server_data.grpc.pb.h"    // ���ݿ����
#include "login_server.h"   // ��¼�������ӿ�

#include <grpcpp/grpcpp.h>
#include <map>
#include <string>


// ��¼��������
class LoginService {
public:
    // ������һ��ָ��Channel������ָ�룬������Զ��ͨ��
    LoginService(std::shared_ptr<grpc::Channel> channel);

	// �����¼����ע������������֤����
    std::string login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query);

private:
    std::unique_ptr<myproject::DatabaseServer::Stub> stub_;
};

class RegisterService {
public:
    // ������һ��ָ��Channel������ָ�룬������Զ��ͨ��
    RegisterService(std::shared_ptr<grpc::Channel> channel);

    // �����¼����ע������������֤����
    std::string register_(const std::string& database, const std::string& table, std::map<std::string, std::string> data);

private:
    std::unique_ptr<myproject::DatabaseServer::Stub> stub_;
};

class AuthenticateService {
public:
    // ������һ��ָ��Channel������ָ�룬������Զ��ͨ��
    AuthenticateService();

    // �����¼����ע������������֤����
    std::string authenticate_(const std::string& token);
};
#endif // !LOGIN_SERVICE_H