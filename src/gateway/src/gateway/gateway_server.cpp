#include "gateway_server.h"
#include <thread>
#include <chrono>

// GatewayServerImpl 构造函数
GatewayServerImpl::GatewayServerImpl(LoggerManager& logger_manager_):
    logger_manager(logger_manager_),	// 日志管理器   
    central_stub(myproject::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))), // 中心服务器存根
    login_connection_pool(10) // 初始化登录服务器连接池，设置连接池大小为10
{
    // 启动定时任务，定时向中心服务器获取最新的连接池状态
}

// 注册服务器
void GatewayServerImpl::register_server()
{
    // 请求
    myproject::RegisterServerRequest request;
    request.set_server_type(myproject::ServerType::GATEWAY);
    request.set_address("127.0.0.1");
    request.set_port("50051");
    // 响应
    myproject::RegisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->RegisterServer(&context, request, &response);

    if (status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("网关服务器注册成功: {} {}","localhost","50051");
        init_connection_pool(); // 初始化连接池
    }
    else
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("网关服务器注册成功: {} {}","localhost","50051");
    }
}

// 注销服务器
void GatewayServerImpl::unregister_server()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    myproject::UnregisterServerRequest request;
    request.set_server_type(myproject::ServerType::GATEWAY);
    request.set_address("localhost");
    request.set_port("50051");

    // 响应
    myproject::UnregisterServerResponse response;

    grpc::Status status = central_stub->UnregisterServer(&context, request, &response);

    if (status.ok() && response.success()) {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("网关服务器注销成功: {} {}","localhost","50051");
    }
    else {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("网关服务器注销失败: {} {}","localhost","50051");
    }
}

// 初始化连接池
void GatewayServerImpl::init_connection_pool()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    myproject::ConnectPoorRequest request;
    request.set_server_type(myproject::ServerType::LOGIN);
    // 响应
    myproject::ConnectPoorResponse response;

    grpc::Status status = central_stub->GetConnectPoor(&context, request, &response);

    if (status.ok())
    {
        // 更新登录服务器连接池
        for (const auto& server_info : response.connect_info())
        {
            login_connection_pool.add_server(myproject::ServerType::LOGIN, server_info.address(), std::to_string(server_info.port()));
        }
    }
    else
    {
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("无法获取连接池信息");
    }
}

// 服务转发接口
grpc::Status GatewayServerImpl::RequestForward(grpc::ServerContext* context, const myproject::ForwardRequest* request, myproject::ForwardResponse* response)
{

    switch (request->service_type()) // 根据请求的服务类型进行转发
    {
    case myproject::ServiceType::REQ_LOGIN: // 用户登录请求
    {
        return forward_to_login_service(request->payload(), response);  
    }
    default:    // 未知服务类型
        response->set_success(false);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Unknown service type");
    }
}

// Login 方法，处理登录请求
grpc::Status GatewayServerImpl::forward_to_login_service(const std::string& payload, myproject::ForwardResponse* response)
{
    myproject::LoginRequest login_request;  // 创建登录请求对象
    bool request_out = login_request.ParseFromString(payload); // 将负载解析为登录请求对象
    if (!request_out) // 如果解析失败
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Failed to parse LoginRequest");
    }

    // 构造响应
    myproject::LoginResponse login_response;
    grpc::ClientContext context;

    // 获取连接池中的连接
    auto channel = login_connection_pool.get_connection(myproject::ServerType::LOGIN);
    auto login_stub = myproject::LoginServer::NewStub(channel);

    grpc::Status status = login_stub->Login(&context, login_request, &login_response);

    if (!status.ok()) // 如果调用失败
    {
        return status;
    }

    bool response_out = login_response.SerializeToString(response->mutable_response()); // 将登录响应序列化为转发响应
    if (!response_out) // 如果序列化失败
    {
        return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to serialize LoginResponse");
    }

    response->set_success(true);
    return grpc::Status::OK;
}
