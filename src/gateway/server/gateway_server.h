#ifndef GATEWAY_SERVER_H
#define GATEWAY_SERVER_H

#include "base_server.h"            // 已包含: common.pb.h, connection_pool.h, logger_manager.h, grpcpp
#include "server_gateway.grpc.pb.h"
#include "server_login.grpc.pb.h"
#include "server_file.grpc.pb.h"
#include "server_db.grpc.pb.h"
#include "redis_client.h"
#include "skynet_tcp_manager.h"     // Skynet TCP 管理器（统一双向通信）

#include <memory>
#include <atomic>
#include <mutex>
#include <thread>

/**
 * @brief 网关服务器实现类
 */
class GatewayServerImpl final : public BaseServer, public rpc_server::GatewayServer::Service
{
public:
    GatewayServerImpl(LoggerManager& logger_manager_, const std::string address, const std::string port);
    ~GatewayServerImpl() override;

    // gRPC 对外接口
    grpc::Status Client_register(grpc::ServerContext* context, const rpc_server::ClientRegisterReq* req, rpc_server::ClientRegisterRes* res) override;  // 客户端注册
    grpc::Status Client_heartbeat(grpc::ServerContext* context, const rpc_server::ClientHeartbeatReq* req, rpc_server::ClientHeartbeatRes* res) override;   // 接收客户端心跳包
    grpc::Status Get_gateway_pool(grpc::ServerContext* context, const rpc_server::GetGatewayPoolReq* req, rpc_server::GetGatewayPoolRes* res) override; // 获取网关服务器连接池
    grpc::Status Request_forward(grpc::ServerContext* context, const rpc_server::ForwardReq* req, rpc_server::ForwardRes* res) override;    // 转发请求

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
    
    // 数据库服务器转发（仅限内部 TCP 调用）
    grpc::Status Forward_to_db_create_service(const std::string& payload, rpc_server::ForwardRes* res);
    grpc::Status Forward_to_db_read_service(const std::string& payload, rpc_server::ForwardRes* res);
    grpc::Status Forward_to_db_update_service(const std::string& payload, rpc_server::ForwardRes* res);
    grpc::Status Forward_to_db_delete_service(const std::string& payload, rpc_server::ForwardRes* res);
    
    // Skynet 游戏服务转发（客户端 -> Skynet）
    grpc::Status Forward_to_enter_game_service(const std::string& payload, rpc_server::ForwardRes* res);
    grpc::Status Forward_to_leave_game_service(const std::string& payload, rpc_server::ForwardRes* res);
    
    // Skynet 玩家服务转发（客户端 -> Skynet）
    grpc::Status Forward_to_player_action_service(const std::string& payload, rpc_server::ForwardRes* res);
    grpc::Status Forward_to_player_status_service(const std::string& payload, rpc_server::ForwardRes* res);
    
    // Skynet 测试服务转发
    grpc::Status Forward_to_echo_service(const std::string& payload, rpc_server::ForwardRes* res);
    
    // 定时任务
    void Update_connection_pool();
    
    // Skynet 请求处理（由 SkynetTcpManager 回调，Skynet -> Gateway）
    std::string Handle_skynet_request(uint16_t msg_type, const std::string& payload);
    
        // Skynet 推送消息处理
        void Handle_skynet_push(const SkynetTcpMessage& msg);

    private:
        RedisClient redis_client;
    
        // 各服务器连接池
            ConnectionPool login_connection_pool;
            ConnectionPool file_connection_pool;
            ConnectionPool gateway_connection_pool;
            ConnectionPool db_connection_pool;
    
            // 定时更新连接池的线程
            std::thread update_pool_thread_;
            std::atomic<bool> pool_update_running_{false};
    
            // Skynet TCP 管理器（统一服务器和客户端功能）
            std::unique_ptr<SkynetTcpManager> skynet_tcp_manager_;
        };

        #endif // GATEWAY_SERVER_H
