#include "gateway_server.h"
#include "../common/gateway_forward_helpers.h"
#include "server_db.grpc.pb.h"

// ==================== 数据库服务转发 ====================
// 数据库添加请求转发
grpc::Status GatewayServerImpl::Forward_to_db_create_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::CreateReq, rpc_server::CreateRes, rpc_server::DBServer>(
        payload,
        res,
        db_connection_pool,
        rpc_server::ServerType::DB,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->Create(context, req, resp);
        },
        "db create",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}

// 数据库查询请求转发
grpc::Status GatewayServerImpl::Forward_to_db_read_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::ReadReq, rpc_server::ReadRes, rpc_server::DBServer>(
        payload,
        res,
        db_connection_pool,
        rpc_server::ServerType::DB,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->Read(context, req, resp);
        },
        "db read",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}

// 数据库更新请求转发
grpc::Status GatewayServerImpl::Forward_to_db_update_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::UpdateReq, rpc_server::UpdateRes, rpc_server::DBServer>(
        payload,
        res,
        db_connection_pool,
        rpc_server::ServerType::DB,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->Update(context, req, resp);
        },
        "db update",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}

// 数据库删除请求转发
grpc::Status GatewayServerImpl::Forward_to_db_delete_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::DeleteReq, rpc_server::DeleteRes, rpc_server::DBServer>(
        payload,
        res,
        db_connection_pool,
        rpc_server::ServerType::DB,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->Delete(context, req, resp);
        },
        "db delete",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}
