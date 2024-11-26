#include "login_service.h"
#include "login_server.h"

// 登录服务接口
grpc::Status LoginServerImpl::Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response)
{
    // 获取用户名和密码
    std::string username = request->username(); // 从 request 对象中提取用户名和密码
    std::string password = request->password();

	// 创建服务对象，处理登录请求
    LoginService login_service(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials()));

    std::map<std::string, std::string> query = { {"user_name", username}, {"user_password", password} }; // 构造查询条件
	std::string responses = login_service.login_("poor_users", "users", query); // 查询的数据库名，表名，查询条件

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

// 注册服务接口
grpc::Status LoginServerImpl::Register(grpc::ServerContext* context, const myproject::RegisterRequest* request, myproject::RegisterResponse* response)
{
    // 获取用户名和密码
    std::string username = request->username(); // 从 request 对象中提取用户名和密码
    std::string password = request->password();
	std::string email = request->email();

    // 创建服务对象，处理注册请求
    RegisterService register_service(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials()));

    std::map<std::string, std::string> data = { {"user_name", username}, {"user_password", password} }; // 构造新增数据
    std::string responses = register_service.register_("poor_users", "users", data); // 查询的数据库名，表名，新增数据

    if (responses == "增加成功")
    {
        response->set_success(true);    // 设置响应对象 response 的 success 字段为 true
        response->set_message("注册成功");
    }
    else
    {
        response->set_success(false);
        response->set_message("注册失败");
    }

    // 返回gRPC状态
    return grpc::Status::OK;
}

// 令牌验证服务接口
grpc::Status LoginServerImpl::Authenticate(grpc::ServerContext* context, const myproject::AuthenticateRequest* request, myproject::AuthenticateResponse* response)
{
    // 获取用户名和密码
    std::string token = request->token(); // 从 request 对象中提取用户名和密码

	// 创建服务对象，处理令牌验证请求
    AuthenticateService authenticate_server;
    std::string responses = authenticate_server.authenticate_(token);

    if (responses == "验证正确")
    {
        response->set_success(true);    // 设置响应对象 response 的 success 字段为 true
        response->set_message("验证正确");
    }
    else
    {
        response->set_success(false);
        response->set_message("验证错误");
    }

    // 返回gRPC状态
    return grpc::Status::OK;
}