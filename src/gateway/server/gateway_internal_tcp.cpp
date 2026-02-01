#include "gateway_server.h"
#include "server_db.grpc.pb.h"

// ==================== Skynet 请求处理 ====================
// 由 SkynetTcpManager 回调，根据消息类型转发到对应服务

std::string GatewayServerImpl::Handle_skynet_request(uint16_t msg_type, const std::string& payload)
{
    rpc_server::ForwardRes response;
    
    switch (static_cast<rpc_server::ServiceType>(msg_type))
    {
    case rpc_server::ServiceType::REQ_DB_CREATE:
        Forward_to_db_create_service(payload, &response);
        break;
    case rpc_server::ServiceType::REQ_DB_READ:
        Forward_to_db_read_service(payload, &response);
        break;
    case rpc_server::ServiceType::REQ_DB_UPDATE:
        Forward_to_db_update_service(payload, &response);
        break;
    case rpc_server::ServiceType::REQ_DB_DELETE:
        Forward_to_db_delete_service(payload, &response);
        break;
    default:
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn(
            "Unknown Skynet message type: {}", msg_type);
        response.set_success(false);
        response.set_response("");
        break;
    }
    
    return response.response();
}

// ==================== Skynet 推送消息处理 ====================
// 处理来自 Skynet 的主动推送消息
// 注意：推送消息类型 (200+) 需要在 common.proto 的 ServiceType 中定义

void GatewayServerImpl::Handle_skynet_push(const SkynetTcpMessage& msg)
{
    // 推送消息类型暂时使用硬编码值，后续可添加到 ServiceType
    constexpr uint16_t PUSH_GAME_STATE = 200;
    constexpr uint16_t PUSH_NOTIFICATION = 201;
    
    switch (msg.msg_type)
    {
    case PUSH_GAME_STATE:
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info(
            "Received game state push from Skynet, len={}", msg.payload.size());
        // TODO: 转发给客户端
        break;
        
    case PUSH_NOTIFICATION:
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info(
            "Received notification push from Skynet, len={}", msg.payload.size());
        // TODO: 转发给客户端
        break;
        
    default:
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn(
            "Unknown Skynet push type: {}", msg.msg_type);
        break;
    }
}
