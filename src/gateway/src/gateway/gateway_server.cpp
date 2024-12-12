#include "gateway_server.h"

#include "server_login.grpc.pb.h"
#include "server_logic.grpc.pb.h"
#include "server_central.grpc.pb.h"

// GatewayServerImpl 构造函数
GatewayServerImpl::GatewayServerImpl() 
{
    // 中心服务器
    central_channel = grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials());
    central_stub = myproject::CentralServer::NewStub(central_channel);
	// 登录服务器
	login_channel = grpc::CreateChannel("localhost:50053", grpc::InsecureChannelCredentials());
	login_stub = myproject::LoginServer::NewStub(login_channel);
}

grpc::Status GatewayServerImpl::RequestForward(grpc::ServerContext* context, const myproject::ForwardRequest* request, myproject::ForwardResponse* response)
{
	switch (request->service_type()) // 根据请求的服务类型进行转发
    {
    case myproject::ServiceType::LOGIN: // 用户登录
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

    // 调用登录服务并获取响应
    myproject::LoginResponse login_response;
    grpc::ClientContext context;
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
