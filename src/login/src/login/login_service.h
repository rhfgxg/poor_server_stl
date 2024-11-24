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
	// �����¼���󣺲�ѯ���ݿ⣬��ѯ��¼���
    std::string login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query);

private:
    std::unique_ptr<myproject::DatabaseService::Stub> stub_;
};

#endif // !LOGIN_SERVICE_H