#include "login_service.h"

grpc::Status LoginServiceImpl::Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) 
{
    /* 
    * 参数列表
    * grpc::ServerContext* context：gRPC 框架提供的上下文对象，包含 RPC 调用的元数据和其他信息。
    * const myproject::LoginRequest* request：客户端发送的登录请求，包含用户名和密码。
    * myproject::LoginResponse* response：服务端返回的响应，包含登录结果和消息。
    * 返回值：
    * gRPC 操作结果
     */

    // 获取用户名和密码
    std::string username = request->username(); // 从 request 对象中提取用户名和密码
    std::string password = request->password();

    // 假设数据库查询和校验
    if (username == "user" && password == "pass") 
    {
		response->set_success(true);    // 设置响应对象 response 的 success 字段为 true
        response->set_message("登录成功");
    }
    else 
    {
        response->set_success(false);
        response->set_message("登录失败");
    }

    // 返回gRPC状态
    return grpc::Status::OK;
}