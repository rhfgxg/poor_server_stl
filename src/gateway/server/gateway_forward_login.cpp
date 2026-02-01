#include "gateway_server.h"
#include "../common/gateway_forward_helpers.h"
#include "server_login.grpc.pb.h"

// ==================== 登录服务转发 ====================
// 用户注册
grpc::Status GatewayServerImpl::Forward_to_register_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::RegisterReq, rpc_server::RegisterRes, rpc_server::LoginServer>(
        payload,
        res,
        login_connection_pool,
        rpc_server::ServerType::LOGIN,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->Register(context, req, resp);
        },
        "user register",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}

// 用户登录
grpc::Status GatewayServerImpl::Forward_to_login_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::LoginReq, rpc_server::LoginRes, rpc_server::LoginServer>(
        payload,
        res,
        login_connection_pool,
        rpc_server::ServerType::LOGIN,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->Login(context, req, resp);
        },
        "user login",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}

// 用户登出
grpc::Status GatewayServerImpl::Forward_to_logout_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::LogoutReq, rpc_server::LogoutRes, rpc_server::LoginServer>(
        payload,
        res,
        login_connection_pool,
        rpc_server::ServerType::LOGIN,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->Logout(context, req, resp);
        },
        "user logout",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}

// 用户修改密码
grpc::Status GatewayServerImpl::Forward_to_change_password_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::ChangePasswordReq, rpc_server::ChangePasswordRes, rpc_server::LoginServer>(
        payload,
        res,
        login_connection_pool,
        rpc_server::ServerType::LOGIN,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->Change_password(context, req, resp);
        },
        "user change password",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}
