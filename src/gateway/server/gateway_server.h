#ifndef GATEWAY_SERVER_H
#define GATEWAY_SERVER_H

#include "base_server.h"            // 已包含: common.pb.h, connection_pool.h, logger_manager.h, grpcpp
#include "server_gateway.grpc.pb.h"
#include "server_login.grpc.pb.h"
#include "server_file.grpc.pb.h"
#include "redis_client.h"

#include <memory>
#include <atomic>

/**
 * @brief 网关服务器实现类（重构版本）
 * 
 * 继承自 BaseServer，自动获得：
 * - 线程池管理（ThreadManager）
 * - 服务器注册/注销
 * - 心跳发送
 * - 日志管理
 * 
 * 只需实现业务逻辑：
 * - 客户端注册和心跳
 * - 请求转发到其他服务器
 * - 连接池管理
 */
class GatewayServerImpl final : public BaseServer, public rpc_server::GatewayServer::Service
{
public:
    GatewayServerImpl(LoggerManager& logger_manager_, const std::string address, const std::string port);
    ~GatewayServerImpl() override;

    // gRPC 对外接口
    grpc::Status Client_register(grpc::ServerContext* context, const rpc_server::ClientRegisterReq* req, rpc_server::ClientRegisterRes* res) override;
    grpc::Status Client_heartbeat(grpc::ServerContext* context, const rpc_server::ClientHeartbeatReq* req, rpc_server::ClientHeartbeatRes* res) override;
    grpc::Status Get_gateway_pool(grpc::ServerContext* context, const rpc_server::GetGatewayPoolReq* req, rpc_server::GetGatewayPoolRes* res) override;
    grpc::Status Request_forward(grpc::ServerContext* context, const rpc_server::ForwardReq* req, rpc_server::ForwardRes* res) override;

protected:
    // BaseServer 钩子方法
    bool on_start() override;
    void on_stop() override;
    void on_registered(bool success) override;

private:
    // 初始化
    void Init_connection_pool();
    
    // 请求转发处理函数
    grpc::Status Handle_return_gateway_poor(const rpc_server::GetGatewayPoolReq* req, rpc_server::GetGatewayPoolRes* res);
    
    // 登录服务器转发
    grpc::Status Forward_to_register_service(const std::string& payload, rpc_server::ForwardRes* res);
    grpc::Status Forward_to_login_service(const std::string& payload, rpc_server::ForwardRes* res);
    grpc::Status Forward_to_logout_service(const std::string& payload, rpc_server::ForwardRes* res);
    grpc::Status Forward_to_change_password_service(const std::string& payload, rpc_server::ForwardRes* res);
    
    // 文件服务器转发
    grpc::Status Forward_to_file_transmission_ready_service(const std::string& payload, rpc_server::ForwardRes* res);
    grpc::Status Forward_to_file_delete_service(const std::string& payload, rpc_server::ForwardRes* res);
    grpc::Status Forward_to_file_list_service(const std::string& payload, rpc_server::ForwardRes* res);
    
    // 定时任务
    void Update_connection_pool();

private:
    RedisClient redis_client;
    
    // 各服务器连接池
    ConnectionPool login_connection_pool;
    ConnectionPool file_connection_pool;
    ConnectionPool gateway_connection_pool;
    
    // 定时更新连接池的线程
    std::thread update_pool_thread_;
    std::atomic<bool> pool_update_running_{false};
};

#endif // GATEWAY_SERVER_H
