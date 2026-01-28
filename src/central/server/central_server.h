#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H

#include "base_server.h"            // 已包含: common.pb.h, connection_pool.h, logger_manager.h, grpcpp
#include "server_central.grpc.pb.h"
#include "server_db.grpc.pb.h"
#include "server_file.grpc.pb.h"
#include "server_gateway.grpc.pb.h"
#include "server_login.grpc.pb.h"

#include <thread>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <atomic>

// 心跳记录
struct HeartbeatRecord
{
    rpc_server::ServerType server_type = rpc_server::ServerType::UNKNOWN;  // 服务器类型
    std::string address;    // 服务器地址
    std::string port;   // 服务器端口
    std::chrono::steady_clock::time_point last_heartbeat;   // 最后一次心跳时间
};

/**
 * @brief 中心服务器实现类
 */
class CentralServerImpl final : public BaseServer, public rpc_server::CentralServer::Service
{
public:
    CentralServerImpl(LoggerManager& logger_manager_, const std::string& address, const std::string& port);
    ~CentralServerImpl() override;

    // grpc对外接口
    // 服务器注册
    grpc::Status Register_server(grpc::ServerContext* context, const rpc_server::RegisterServerReq* req, rpc_server::RegisterServerRes* res) override;
    // 服务器断开
    grpc::Status Unregister_server(grpc::ServerContext* context, const rpc_server::UnregisterServerReq* req, rpc_server::UnregisterServerRes* res) override;
    // 获取连接池中所有链接
    grpc::Status Get_connec_poor(grpc::ServerContext* context, const rpc_server::MultipleConnectPoorReq* req, rpc_server::MultipleConnectPoorRes* res) override;
    // 接收心跳包
    grpc::Status Heartbeat(grpc::ServerContext* context, const rpc_server::HeartbeatReq* req, rpc_server::HeartbeatRes* res) override;

protected:
    // BaseServer 钩子方法
    bool on_start() override;
    void on_stop() override;

private:
    // grpc工具函数
    // 执行注册服务器
    void Handle_register_server(const rpc_server::RegisterServerReq* req, rpc_server::RegisterServerRes* res);
    // 释放连接池中服务器连接
    void Release_server_connection(rpc_server::ServerType server_type, const std::string& address, const std::string& port);
    // 返回连接池中的连接
    void All_connec_poor(const rpc_server::MultipleConnectPoorReq* req, rpc_server::MultipleConnectPoorRes* res);

    // 获取对应类型的连接池
    ConnectionPool* get_connection_pool(rpc_server::ServerType server_type);
    // 获取连接池标签
    const char* get_pool_label(rpc_server::ServerType server_type) const;

    // 心跳检查
    void check_heartbeat_worker();  // 心跳检查工作线程
    void start_heartbeat_checker(); // 启动心跳检查器
    void stop_heartbeat_checker();  // 停止心跳检查器

private:
    // 心跳记录
    std::unordered_map<std::string, HeartbeatRecord> heartbeat_records_;    // 心跳记录映射（key: address）
    std::mutex heartbeat_mutex_;    // 保护 heartbeat_records_ 的互斥锁
    std::thread heartbeat_checker_thread_;  // 心跳检查线程
    std::atomic<bool> heartbeat_checker_running_;   // 心跳检查运行标志

    // 连接池（管理其他服务器）
    ConnectionPool central_connection_pool_;    // 管理中心服务器连接池
    ConnectionPool db_connection_pool_;   // 数据库服务器连接池
    ConnectionPool redis_connection_pool_;  // Redis 服务器连接池
    ConnectionPool file_connection_pool_;   // 文件服务器连接池
    ConnectionPool gateway_connection_pool_;    // 网关服务器连接池
    ConnectionPool login_connection_pool_;  // 登录服务器连接池
};

#endif // CENTRAL_SERVER_H
