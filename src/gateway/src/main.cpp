#include "gateway/gateway_server.h"
#include "logger_manager.h" // 引入日志管理器

// 运行服务器
void RunServer(LoggerManager& logger_manager);
// 模拟客户端登录
void test_client();

// 网关服务器main函数
int main()
{
    // 初始化日志管理器，通过引用传递实现单例模式
    LoggerManager logger_manager;
    logger_manager.initialize(myproject::ServerType::GATEWAY);    // 传入服务器类型，创建日志文件夹

    // 记录启动日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Gateway_server started"); // 记录启动日志：日志分类, 日志内容

    RunServer(logger_manager); // 运行服务器

    return 0; // 返回0表示程序正常结束
}

void RunServer(LoggerManager& logger_manager)
{
    GatewayServerImpl gateway_server(logger_manager); // 网关服务器实现

    grpc::ServerBuilder builder; // gRPC服务器构建器
    std::string server_address("0.0.0.0:50051"); // 网关服务器监听50051端口
    builder.AddListeningPort(server_address,grpc::InsecureServerCredentials()); // 添加监听端口
    builder.RegisterService(&gateway_server); // 注册服务

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // 构建并启动服务器
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Listening address: {}",server_address);

    gateway_server.start_thread_pool(4); // 启动4个线程处理请求

    test_client();  // 模拟客户端登录

    server->Wait(); // 等待服务器终止

    gateway_server.stop_thread_pool(); // 停止线程池
}

void test_client()
{
    auto channel = grpc::CreateChannel("localhost:50051",grpc::InsecureChannelCredentials()); // 链接网关服务器
    auto stub = myproject::GatewayServer::NewStub(channel); // 服务存根

    // 模拟登录数据
    std::string user_name = "lhw";
    std::string password = "159357";

    // 构造登录请求
    myproject::LoginRequest login_request;
    login_request.set_username(user_name); // 设置用户名
    login_request.set_password(password); // 设置密码

    // 包装为转发请求
    myproject::ForwardRequest forward_request;
    forward_request.set_service_type(myproject::ServiceType::REQ_LOGIN); // 设置服务类型
    login_request.SerializeToString(forward_request.mutable_payload()); // 序列化登录请求

    //for(int i = 0; i < 100; ++i)
    {
        // 构造响应
        myproject::ForwardResponse forward_response;
        grpc::ClientContext client_context; // 包含 RPC 调用的元数据和其他信息

        // 向网关服务器发送请求
        grpc::Status status = stub->RequestForward(&client_context,forward_request,&forward_response);

        if(status.ok() && forward_response.success())
        {
            std::cout << "login yes" << std::endl;
        }
        else
        {
            std::cout << "login no" << std::endl;
        }
    }
}
