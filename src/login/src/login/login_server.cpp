#include "login_service.h"
#include "login_server.h"

grpc::Status LoginServiceImpl::Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response)
{
    // 获取用户名和密码
    std::string username = request->username(); // 从 request 对象中提取用户名和密码
    std::string password = request->password();

	// 创建服务对象，处理登录请求
    LoginService login(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials()));

    std::map<std::string, std::string> query = { {"user_name", username}, {"user_password", password} }; // 构造查询条件
	std::string responses = login.login_("poor_users", "users", query); // 查询的数据库名，表名，查询条件

    if (responses == "登录成功")
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