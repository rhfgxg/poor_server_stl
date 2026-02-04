#include "gateway_server.h"

// ==================== Skynet 逻辑服务器转发 ====================
// 统一处理所有转发到 Skynet Logic Server 的请求
// 通信协议：[4字节长度][2字节类型][数据]
// 消息类型使用 ServiceType 枚举

/**
 * @brief 内部转发函数 - 向 Skynet 发送请求并等待响应
 * @param manager Skynet TCP 管理器
 * @param msg_type 消息类型 (ServiceType)
 * @param payload 请求数据（透传）
 * @param res 响应结果
 * @param logger 日志记录器
 * @param operation_name 操作名称（用于日志）
 * @return gRPC 状态
 */
static grpc::Status forward_to_skynet(
    SkynetTcpManager* manager,
    uint16_t msg_type,
    const std::string& payload,
    rpc_server::ForwardRes* res,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& operation_name)
{
    if (!manager)
    {
        logger->error("{}: Skynet TCP manager not initialized", operation_name);
        res->set_success(false);
        return grpc::Status::OK;
    }
    
    // 检查连接状态，必要时尝试连接
    if (!manager->is_connected())
    {
        // TODO: 从配置文件读取 Skynet 地址
        const std::string skynet_host = "127.0.0.1";
        const uint16_t skynet_port = 8888;  // 与 Skynet cpp_gateway 监听端口一致
        
        if (!manager->connect(skynet_host, skynet_port))
        {
            logger->error("{}: Failed to connect to Skynet at {}:{}", 
                         operation_name, skynet_host, skynet_port);
            res->set_success(false);
            return grpc::Status::OK;
        }
    }
    
    // 构造请求并发送（透传 payload）
    SkynetTcpMessage request(msg_type, payload);
    SkynetTcpMessage response;
    
    if (manager->send_and_wait(request, response, 5000))
    {
        res->set_success(true);
        res->set_response(response.payload);
        logger->debug("{}: success, response_len={}", operation_name, response.payload.size());
    }
    else
    {
        res->set_success(false);
        logger->warn("{}: timeout or failed", operation_name);
    }
    
    return grpc::Status::OK;
}

// ==================== 游戏会话服务 ====================

grpc::Status GatewayServerImpl::Forward_to_enter_game_service(
    const std::string& payload, 
    rpc_server::ForwardRes* res)
{
    return forward_to_skynet(
        skynet_tcp_manager_.get(),
        static_cast<uint16_t>(rpc_server::ServiceType::REQ_ENTER_GAME),
        payload,
        res,
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY),
        "enter_game"
    );
}

grpc::Status GatewayServerImpl::Forward_to_leave_game_service(
    const std::string& payload, 
    rpc_server::ForwardRes* res)
{
    return forward_to_skynet(
        skynet_tcp_manager_.get(),
        static_cast<uint16_t>(rpc_server::ServiceType::REQ_LEAVE_GAME),
        payload,
        res,
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY),
        "leave_game"
    );
}

// ==================== 玩家操作服务 ====================

grpc::Status GatewayServerImpl::Forward_to_player_action_service(
    const std::string& payload, 
    rpc_server::ForwardRes* res)
{
    return forward_to_skynet(
        skynet_tcp_manager_.get(),
        static_cast<uint16_t>(rpc_server::ServiceType::REQ_PLAYER_ACTION),
        payload,
        res,
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY),
        "player_action"
    );
}

grpc::Status GatewayServerImpl::Forward_to_player_status_service(
    const std::string& payload, 
    rpc_server::ForwardRes* res)
{
    return forward_to_skynet(
        skynet_tcp_manager_.get(),
        static_cast<uint16_t>(rpc_server::ServiceType::REQ_GET_PLAYER_STATUS),
        payload,
        res,
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY),
        "get_player_status"
    );
}

// ==================== 测试服务 ====================

grpc::Status GatewayServerImpl::Forward_to_echo_service(
    const std::string& payload, 
    rpc_server::ForwardRes* res)
{
    auto logger = get_logger(poor::LogCategory::APPLICATION_ACTIVITY);
    
    if (!skynet_tcp_manager_)
    {
        logger->error("echo: Skynet TCP manager not initialized");
        res->set_success(false);
        return grpc::Status::OK;
    }
    
    // 检查连接状态
    if (!skynet_tcp_manager_->is_connected())
    {
        const std::string skynet_host = "127.0.0.1";
        const uint16_t skynet_port = 8888;
        
        if (!skynet_tcp_manager_->connect(skynet_host, skynet_port))
        {
            logger->error("echo: Failed to connect to Skynet at {}:{}", 
                         skynet_host, skynet_port);
            res->set_success(false);
            return grpc::Status::OK;
        }
    }
    
    // 构造请求并发送
    SkynetTcpMessage request(rpc_server::ServiceType::REQ_ECHO, payload);
    SkynetTcpMessage response;
    
    logger->info("echo: Sending to Skynet: {}", payload);
    
    if (skynet_tcp_manager_->send_and_wait(request, response, 5000))
    {
        res->set_success(true);
        res->set_response(response.payload);
        logger->info("echo: Response from Skynet: {}", response.payload);
    }
    else
    {
        res->set_success(false);
        logger->warn("echo: Timeout or failed");
    }
    
    return grpc::Status::OK;
}
