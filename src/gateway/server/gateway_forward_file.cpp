#include "gateway_server.h"
#include "../common/gateway_forward_helpers.h"
#include "server_file.grpc.pb.h"

// ==================== 文件服务转发 ====================

grpc::Status GatewayServerImpl::Forward_to_file_transmission_ready_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::TransmissionReadyReq, rpc_server::TransmissionReadyRes, rpc_server::FileServer>(
        payload,
        res,
        file_connection_pool,
        rpc_server::ServerType::FILE,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->Transmission_ready(context, req, resp);
        },
        "file transmission ready",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}

grpc::Status GatewayServerImpl::Forward_to_file_delete_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::DeleteFileReq, rpc_server::DeleteFileRes, rpc_server::FileServer>(
        payload,
        res,
        file_connection_pool,
        rpc_server::ServerType::FILE,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->Delete(context, req, resp);
        },
        "file delete",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}

grpc::Status GatewayServerImpl::Forward_to_file_list_service(const std::string& payload, rpc_server::ForwardRes* res)
{
    return gateway::forward_request<rpc_server::ListFilesReq, rpc_server::ListFilesRes, rpc_server::FileServer>(
        payload,
        res,
        file_connection_pool,
        rpc_server::ServerType::FILE,
        [](auto* stub, auto* context, const auto& req, auto* resp) {
            return stub->ListFiles(context, req, resp);
        },
        "file list",
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)
    );
}
