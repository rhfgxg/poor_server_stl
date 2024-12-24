#include "login_server.h"

LoginServerImpl::LoginServerImpl(LoggerManager& logger_manager_):
    logger_manager(logger_manager_),	// 日志管理器
    central_stub(myproject::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))), // 链接中心服务器
	db_connection_pool(10) // 初始化数据库服务器连接池，设置连接池大小为10
{

}

// 注册服务器
void LoginServerImpl::register_server() 
{
    // 请求
    myproject::RegisterServerRequest request;
    request.set_server_type(myproject::ServerType::LOGIN);
    request.set_address("127.0.0.1");
    request.set_port("50053");

    // 响应
    myproject::RegisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->RegisterServer(&context, request, &response);

    if (status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("登录服务器注册成功: {} {}","localhost","50053");

		init_connection_pool(); // 初始化连接池
    }
    else 
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("登录服务器注册失败: {} {}","localhost","50053");
    }
}

// 注销服务器
void LoginServerImpl::unregister_server() 
{
    // 请求
    myproject::UnregisterServerRequest request;
    request.set_server_type(myproject::ServerType::LOGIN);
    request.set_address("localhost");
    request.set_port("50053");

    // 响应
    myproject::UnregisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->UnregisterServer(&context, request, &response);

    if (status.ok() && response.success()) {
        std::cout << "服务器注销成功: " << response.message() << std::endl;
    }
    else {
        std::cerr << "服务器注销失败: " << response.message() << std::endl;
    }
}

// 初始化连接池
void LoginServerImpl::init_connection_pool()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    myproject::ConnectPoorRequest request;
    request.set_server_type(myproject::ServerType::DATA);
    // 响应
    myproject::ConnectPoorResponse response;

    grpc::Status status = central_stub->GetConnectPoor(&context, request, &response);

    if (status.ok())
    {
        // 更新登录服务器连接池
        for (const auto& server_info : response.connect_info())
        {
            db_connection_pool.add_server(myproject::ServerType::DATA, server_info.address(), std::to_string(server_info.port()));
        }
    }
    else
    {
        std::cerr << "无法获取登录服务器连接池信息: " << status.error_message() << std::endl;
    }
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

    // 获取连接池中的连接
    auto channel = db_connection_pool.get_connection(myproject::ServerType::DATA);
    auto db_stub = myproject::DatabaseServer::NewStub(channel);

    // 向数据库服务器发送查询请求
    grpc::Status status = db_stub->Read(&client_context, read_request, &read_response);

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
