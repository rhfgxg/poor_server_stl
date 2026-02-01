#include "gateway_server.h"

// ==================== Skynet 测试服务转发 ====================
// 简单的 Echo 测试：发送字符串，Skynet 打印并返回

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
    
    // 检查连接状态，必要时尝试连接
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
