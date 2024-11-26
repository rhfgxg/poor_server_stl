#include "./login_service.h"

/********************************************* 登录服务 ************************************************************/
// 构造函数，初始化 stub_
LoginService::LoginService(std::shared_ptr<grpc::Channel> channel) : 
    stub_(myproject::DatabaseServer::NewStub(channel))
{

}

// Login 方法，处理登录请求
std::string LoginService::login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query)
{
    myproject::ReadRequest request;    // 传入参数
    request.set_database(database); // 设置查询数据库
	request.set_table(table); // 设置查询表
	for (auto& it : query)
	{
		(*request.mutable_query())[it.first] = it.second; // 设置查询条件
	}

    myproject::ReadResponse response;  // 返回参数
    grpc::ClientContext context;    // 包含 RPC 调用的元数据和其他信息。

    grpc::Status status = stub_->Read(&context, request, &response); // 向数据库服务器发送查询请求

    if (status.ok())
    {
        // 处理查询结果
		std::cout << "登录成功" << std::endl;
        return "登录成功"; // 返回响应消息
    }
    else
    {
		std::cout << "登录失败" << std::endl;
        return "登录失败"; // 返回错误信息
    }
}

/********************************************* 注册服务 ************************************************************/
RegisterService::RegisterService(std::shared_ptr<grpc::Channel> channel) :
    stub_(myproject::DatabaseServer::NewStub(channel))
{

}

std::string RegisterService::register_(const std::string& database, const std::string& table, std::map<std::string, std::string> data)
{
    myproject::ReadRequest request;    // 传入参数
    request.set_database(database); // 设置查询数据库
    request.set_table(table); // 设置查询表
    // 设置添加的数据
    for (auto& it : data)
    {
        //(*request.mutable_data())[it.first] = it.second; // 设置添加的数据
    }

    myproject::ReadResponse response;  // 返回参数
    grpc::ClientContext context;    // 包含 RPC 调用的元数据和其他信息。

    grpc::Status status = stub_->Read(&context, request, &response); // 向数据库服务器发送查询请求

    if (status.ok())
    {
        // 处理查询结果
        std::cout << "注册成功" << std::endl;
        return "注册成功"; // 返回响应消息
    }
    else
    {
        std::cout << "注册失败" << std::endl;
        return "注册失败"; // 返回错误信息
    }
}


/******************************************* 令牌验证服务 **********************************************************/
AuthenticateService::AuthenticateService()
{

}

std::string AuthenticateService::authenticate_(const std::string& token)
{
    if (1)
    {
        // 处理查询结果
        std::cout << "登录成功" << std::endl;
        return "登录成功"; // 返回响应消息
    }
    else
    {
        std::cout << "登录失败" << std::endl;
        return "登录失败"; // 返回错误信息
    }
}
