#include "./gateway_service.h"

// GatewayService 构造函数，初始化 stub_
GatewayService::GatewayService(std::shared_ptr<grpc::Channel> channel) : stub_(myproject::LoginService::NewStub(channel)) 
{

}

// Login 方法，处理登录请求
std::string GatewayService::Login(const std::string& username, const std::string& password) 
{
    myproject::LoginRequest request;    // 传入参数
    request.set_username(username); // 设置用户名
    request.set_password(password); // 设置密码

	myproject::LoginResponse response;  // 返回参数
    grpc::ClientContext context;    // 包含 RPC 调用的元数据和其他信息。

    grpc::Status status = stub_->Login(&context, request, &response); // 向登录服务器发送登录请求

    if (status.ok()) 
    {
        return response.message(); // 返回响应消息
    }
    else 
    {
        return "RPC failed"; // 返回错误信息
    }
}
