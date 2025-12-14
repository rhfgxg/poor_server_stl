#ifndef GATEWAY_FORWARD_HELPERS_H
#define GATEWAY_FORWARD_HELPERS_H

#include "common.pb.h"
#include "server_gateway.grpc.pb.h"
#include "connection_pool.h"
#include "logger_manager.h"

#include <grpcpp/grpcpp.h>
#include <memory>
#include <functional>

namespace gateway {

/**
 * @brief 通用转发辅助函数
 * 
 * 这个模板函数处理所有转发请求的通用逻辑：
 * 1. 反序列化请求
 * 2. 获取连接
 * 3. 调用 RPC
 * 4. 序列化响应
 * 5. 释放连接
 * 6. 记录日志
 */
template<typename ReqType, typename ResType, typename StubType>
grpc::Status forward_request(
    const std::string& payload,
    rpc_server::ForwardRes* res,
    ConnectionPool& connection_pool,
    rpc_server::ServerType server_type,
    std::function<grpc::Status(typename StubType::Stub*, grpc::ClientContext*, const ReqType&, ResType*)> rpc_call,
    const std::string& operation_name,
    std::shared_ptr<spdlog::logger> logger)
{
    ReqType request;
    ResType response;
    
    // 1. 反序列化请求
    if (!request.ParseFromString(payload))
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, 
                          "Failed to parse " + operation_name + "Req");
    }
    
    // 2. 获取连接
    auto channel = connection_pool.get_connection(server_type);
    auto stub = StubType::NewStub(channel);
    
    // 3. 调用 RPC
    grpc::ClientContext context;
    grpc::Status status = rpc_call(stub.get(), &context, request, &response);
    
    if (!status.ok())
    {
        connection_pool.release_connection(server_type, channel);
        return status;
    }
    
    // 4. 序列化响应
    if (!response.SerializeToString(res->mutable_response()))
    {
        connection_pool.release_connection(server_type, channel);
        return grpc::Status(grpc::StatusCode::INTERNAL, 
                          "Failed to serialize " + operation_name + "Res");
    }
    
    // 5. 设置成功状态
    res->set_success(response.success());
    
    // 6. 记录日志
    logger->info("Forward REQ successfully: {}", operation_name);
    
    // 7. 释放连接
    connection_pool.release_connection(server_type, channel);
    
    return grpc::Status::OK;
}

} // namespace gateway

#endif // GATEWAY_FORWARD_HELPERS_H
