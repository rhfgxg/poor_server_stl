#ifndef MATCHING_SERVER_H
#define MATCHING_SERVER_H

#include "common.grpc.pb.h"
#include "server_matching.grpc.pb.h"
#include "server_login.grpc.pb.h"
#include "base_server.h"
#include "connection_pool.h"
#include "logger_manager.h"
#include "redis_client.h"

#include <grpcpp/grpcpp.h>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <memory>

// 玩家数据
struct PlayerInfo
{
    int32_t player_id;  // 修改为 int32_t
    int rank;
    int score;
};

/**
 * @brief 匹配服务器实现类（重构版本）
 * 
 * 继承自 BaseServer，自动获得：
 * - 线程池管理（ThreadManager）
 * - 服务器注册/注销
 * - 心跳发送
 * - 日志管理
 * 
 * 只需实现业务逻辑：
 * - 玩家匹配
 * - 匹配队列管理
 * - 取消匹配
 */
class MatchingServerImpl final : public BaseServer, public rpc_server::MatchingServer::Service
{
public:
    MatchingServerImpl(LoggerManager& logger_manager_, const std::string address, std::string port);
    ~MatchingServerImpl() override;

    // gRPC 服务接口
    grpc::Status MatchPlayer(grpc::ServerContext* context, const rpc_server::MatchPlayerReq* req, grpc::ServerWriter<rpc_server::MatchPlayerRes>* writer) override;
    grpc::Status CancelMatch(grpc::ServerContext* context, const rpc_server::CancelMatchReq* req, rpc_server::CancelMatchRes* res) override;

protected:
    // BaseServer 钩子方法
    bool on_start() override;
    void on_stop() override;
    void on_registered(bool success) override;

private:
    // 禁止拷贝
    MatchingServerImpl(MatchingServerImpl const&) = delete;
    MatchingServerImpl& operator=(MatchingServerImpl const&) = delete;

    // 连接池管理
    void Init_connection_pool();
    void Update_connection_pool();

private:
    RedisClient redis_client;
    
    // 连接池
    ConnectionPool login_connection_pool;
    
    // 匹配队列（按段位分组）
    std::unordered_map<int, std::list<PlayerInfo>> match_queues;
    std::mutex match_mutex;
    
    // 定时更新连接池的线程
    std::thread update_pool_thread_;
    std::atomic<bool> pool_update_running_{false};
};

#endif // MATCHING_SERVER_H
