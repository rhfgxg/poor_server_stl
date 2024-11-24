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
	// 处理登录请求：查询数据库，查询登录结果
    std::string login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query);

private:
    std::unique_ptr<myproject::DatabaseService::Stub> stub_;
};

#endif // !LOGIN_SERVICE_H