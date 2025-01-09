#include "gateway/gateway_server.h"
#include "logger_manager.h" // 引入日志管理器
#include <thread>
#include <chrono>
#include <string>

void RunServer(LoggerManager& logger_manager);  // 运行服务器

std::string test_login();  // 模拟客户端登录
void test_register();   // 模拟客户端注册
void test_authenticate(std::string token);    // 模拟客户端令牌验证

// 网关服务器main函数
int main()
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(rpc_server::ServerType::GATEWAY);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Gateway_server started"); // 记录启动日志：日志分类, 日志内容

    RunServer(logger_manager); // 运行服务器

    return 0; // 返回0表示程序正常结束
}

// 运行服务器
void RunServer(LoggerManager& logger_manager)
{
    GatewayServerImpl gateway_server(logger_manager); // 网关服务器实现

    grpc::ServerBuilder builder; // gRPC服务器构建器
    std::string server_address("0.0.0.0:50051"); // 网关服务器监听50051端口
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&gateway_server); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // 构建并启动服务器
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Listening address: {}", server_address);

    gateway_server.start_thread_pool(4); // 启动4个线程处理请求

    std::string token = test_login();  // 模拟客户端登录

    server->Wait(); // 等待服务器终止

    gateway_server.stop_thread_pool(); // 停止线程池
}

// 模拟客户端登录
std::string test_login()
{
    auto channel = grpc::CreateChannel("localhost:50051",grpc::InsecureChannelCredentials()); // 链接网关服务器
    auto stub = rpc_server::GatewayServer::NewStub(channel); // 服务存根

    // 模拟登录数据
    std::string account = "3056078308";
    std::string password = "159357";

    // 构造登录请求
    rpc_server::LoginReq login_req;
    login_req.set_account(account);    // 设置用户账号
    login_req.set_password(password);   // 设置密码

    // 包装为转发请求
    rpc_server::ForwardReq forward_req;
    forward_req.set_service_type(rpc_server::ServiceType::REQ_LOGIN); // 设置服务类型
    login_req.SerializeToString(forward_req.mutable_payload()); // 序列化登录请求


    // 构造响应
    rpc_server::ForwardRes forward_res;
    grpc::ClientContext client_context; // 包含 RPC 调用的元数据和其他信息

    // 向网关服务器发送请求
    grpc::Status status = stub->Request_forward(&client_context,forward_req, &forward_res);

    if(status.ok() && forward_res.success())
    {
        // 获取Token
        rpc_server::LoginRes login_res; // 登录响应类
        login_res.ParseFromString(forward_res.response()); // 从转发响应中解析登录响应
        std::string token = login_res.token(); // 获取Token

        std::cout << login_res.message() << std::endl;  // 输出登录响应信息

        return token;
    }
    else
    {
        std::cout << "Login failed" << std::endl;

        return "";
    }
    
}

// 模拟客户端注册
void test_register()
{
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()); // 链接网关服务器
    auto stub = rpc_server::GatewayServer::NewStub(channel); // 服务存根
    // 模拟注册数据
    std::string user_name = "3056078308";
    std::string password = "159357";
    // 构造注册请求
    rpc_server::RegisterReq register_req;
    register_req.set_user_name(user_name);    // 设置用户账号
    register_req.set_password(password);   // 设置密码
    // 包装为转发请求
    rpc_server::ForwardReq forward_req;
    forward_req.set_service_type(rpc_server::ServiceType::REQ_REGISTER); // 设置服务类型
    register_req.SerializeToString(forward_req.mutable_payload()); // 序列化注册请求
    // 构造响应
    rpc_server::ForwardRes forward_res;
    grpc::ClientContext client_context; // 包含 RPC 调用的元数据和其他信息
    // 向网关服务器发送请求
    grpc::Status status = stub->Request_forward(&client_context, forward_req, &forward_res);
    if(status.ok() && forward_res.success())
    {
        std::cout << "Register successful" << std::endl;
    } else
    {
        std::cout << "Register failed" << std::endl;
    }
}

// 模拟客户端令牌验证
void test_authenticate(std::string token)
{
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()); // 链接网关服务器
    auto stub = rpc_server::GatewayServer::NewStub(channel); // 服务存根

    // 模拟登录数据
    std::string account = "3056078308";

    // 构造登录请求
    rpc_server::AuthenticateReq authenticate_req;
    authenticate_req.set_account(account);    // 设置用户账号
    authenticate_req.set_token(token);   // 设置token

    // 包装为转发请求
    rpc_server::ForwardReq forward_req;
    forward_req.set_service_type(rpc_server::ServiceType::REQ_LOGIN); // 设置服务类型
    authenticate_req.SerializeToString(forward_req.mutable_payload()); // 序列化登录请求

    // 构造响应
    rpc_server::ForwardRes forward_res;
    grpc::ClientContext client_context; // 包含 RPC 调用的元数据和其他信息

    // 向网关服务器发送请求
    grpc::Status status = stub->Request_forward(&client_context, forward_req, &forward_res);

    if(status.ok() && forward_res.success())
    {
        // 获取Token
        rpc_server::AuthenticateRes authenticate_res; // 登录响应类
        authenticate_res.ParseFromString(forward_res.response()); // 从转发响应中解析登录响应

        if(authenticate_res.success())
        {
            std::cout << "token validated" << std::endl;
        } else
        {
            std::cout << "token invalid" << std::endl;
        }
    }
    else
    {
        std::cout << "token failed" << std::endl;
    }

}
