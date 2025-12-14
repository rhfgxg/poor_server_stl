#ifndef REDIS_SERVER_H
#define REDIS_SERVER_H

#include "base_server.h"
#include "redis_cluster_client.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <nlohmann/json.hpp>

// 前置声明（使用现有的 RedisClient）
class RedisClient;

// Redis 服务器类
// 职责：
// 1. 统一管理本地 Redis 和集群 Redis 连接
// 2. 提供 gRPC 接口给其他 C++ 服务（可选）
// 3. 提供 TCP Socket 接口给 Skynet 服务
// 4. 连接池管理和负载均衡
class RedisServer : public BaseServer
{
public:
    RedisServer(LoggerManager& logger_manager, const std::string& address, const std::string& port);
    ~RedisServer() override;

    // BaseServer 钩子方法
    bool on_start() override;
    void on_stop() override;
    
    // 等待停止
    void wait_for_shutdown();

protected:
    // ==================== Redis 连接管理 ====================
    
    // 初始化 Redis 连接
    bool init_redis_connections();
    
    // 获取本地 Redis 客户端（使用现有的 RedisClient）
    std::shared_ptr<RedisClient> get_local_redis();
    
    // 获取集群 Redis 客户端
    std::shared_ptr<RedisClusterClient> get_cluster_redis();

    // ==================== TCP Socket 服务（给 Skynet） ====================
    
    // 启动 Socket 监听（给 Skynet 连接）
    bool start_socket_listener();
    
    // 处理 Socket 连接
    void handle_socket_connection(int client_fd);
    
    // 处理 Socket 消息
    void handle_socket_message(int client_fd, const std::string& message);
    
    // 发送 Socket 响应
    void send_socket_response(int client_fd, const nlohmann::json& response);

    // ==================== gRPC 服务（给 C++ 服务） ====================
    
    // gRPC 服务实现类（可选，暂不实现）
    // class RedisServiceImpl;
    // std::unique_ptr<RedisServiceImpl> grpc_service_;

private:
    // 本地 Redis 连接（使用现有的 RedisClient）
    std::shared_ptr<RedisClient> local_redis_;
    std::shared_mutex local_redis_mutex_;
    
    // 集群 Redis 连接（使用新的 RedisClusterClient）
    std::shared_ptr<RedisClusterClient> cluster_redis_;
    std::shared_mutex cluster_redis_mutex_;
    
    // Socket 监听器
    int socket_fd_;  // Socket 文件描述符
    std::thread socket_listener_thread_;
    std::atomic<bool> running_;  // 运行状态标志
    
    // 客户端连接管理
    std::unordered_map<int, std::thread> client_threads_;
    std::shared_mutex clients_mutex_;
};

#endif // REDIS_SERVER_H
