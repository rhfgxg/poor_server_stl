#include "login_server.h"

LoginServerImpl::LoginServerImpl() : 
	channel_(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials())),   // 链接数据库服务器
    stub_(myproject::DatabaseServer::NewStub(channel_))
{

}

// 登录服务接口
grpc::Status LoginServerImpl::Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response)
{
    // 获取用户名和密码
    std::string username = request->username(); // 从 request 对象中提取用户名和密码
    std::string password = request->password();
	// 构造查询条件
    std::map<std::string, std::string> query = { {"user_name", username}, {"user_password", password} }; 

	std::string responses = login_("poor_users", "users", query); // 查询的数据库名，表名，查询条件

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

    // 返回gRPC状态
    return grpc::Status::OK;
}

// 令牌验证服务接口
grpc::Status LoginServerImpl::Authenticate(grpc::ServerContext* context, const myproject::AuthenticateRequest* request, myproject::AuthenticateResponse* response)
{

    return grpc::Status::OK;
}


// 登录服务
std::string LoginServerImpl::login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query)
{
    // 构造请求
    myproject::ReadRequest read_request;
    read_request.set_database(database); // 设置查询数据库
    read_request.set_table(table); // 设置查询表
    for (auto& it : query)
    {
        (*read_request.mutable_query())[it.first] = it.second; // 设置查询条件
    }

    // 构造响应
    myproject::ReadResponse read_response;
    grpc::ClientContext client_context; // 包含 RPC 调用的元数据和其他信息

    // 向数据库服务器发送查询请求
    grpc::Status status = stub_->Read(&client_context, read_request, &read_response);

    if (status.ok())
    {
        // 打印查询结果
        for (const auto& result : read_response.results())
        {
            std::cout << "查询结果: ";
            for (const auto& field : result.fields())
            {
                std::cout << field.first << ": " << field.second << ", ";
            }
            std::cout << std::endl;
        }

        std::cout << "登录成功" << std::endl;
        return "登录成功";
    }
    else
    {
        std::cout << "登录失败" << std::endl;
        return "登录失败";
    }
}

// 注册服务
std::string LoginServerImpl::register_(const std::string& database, const std::string& table, std::map<std::string, std::string> data)
{
	return "注册成功";
}

// 令牌验证服务
std::string LoginServerImpl::authenticate_(const std::string& token)
{
	return "验证成功";
}