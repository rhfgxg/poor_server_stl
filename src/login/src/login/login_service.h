#ifndef LOGIN_SERVICE_H
#define LOGIN_SERVICE_H

#include "server_data.grpc.pb.h"    // 数据库服务
#include "login_server.h"   // 登录服务器接口

#include <grpcpp/grpcpp.h>
#include <map>
#include <string>


// 登录服务处理类
class LoginService {
public:
    // 参数是一个指向Channel的智能指针，用来和远程通信
    LoginService(std::shared_ptr<grpc::Channel> channel);

	// 处理登录请求，注册请求，令牌验证请求
    std::string login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query);

private:
    std::unique_ptr<myproject::DatabaseServer::Stub> stub_;
};

class RegisterService {
public:
    // 参数是一个指向Channel的智能指针，用来和远程通信
    RegisterService(std::shared_ptr<grpc::Channel> channel);

    // 处理登录请求，注册请求，令牌验证请求
    std::string register_(const std::string& database, const std::string& table, std::map<std::string, std::string> data);

private:
    std::unique_ptr<myproject::DatabaseServer::Stub> stub_;
};

class AuthenticateService {
public:
    // 参数是一个指向Channel的智能指针，用来和远程通信
    AuthenticateService();

    // 处理登录请求，注册请求，令牌验证请求
    std::string authenticate_(const std::string& token);
};
#endif // !LOGIN_SERVICE_H