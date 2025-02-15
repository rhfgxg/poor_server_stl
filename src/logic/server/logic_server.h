#ifndef LOGIC_SERVER_H
#define LOGIC_SERVER_H

#include "common.grpc.pb.h" // 公共模块：包含公共数据类型，枚举
#include "consts/const_log.h"  // 日志类型
#include "server_logic.grpc.pb.h"	// 逻辑服务
#include "server_db.grpc.pb.h"    // 数据库服务
#include "server_central.grpc.pb.h"	// 中心服务
#include "connection_pool.h"    // 连接池
#include "logger_manager.h"     // 日志管理器
#include "redis_client.h"       // Redis客户端
#include "common/achievement/achievement_manager.h"    // 成就管理器

#include <grpcpp/grpcpp.h>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <lua.hpp>

// 逻辑服务器实现类
class LogicServerImpl final: public rpc_server::LogicServer::Service
{
public:
    LogicServerImpl(LoggerManager& logger_manager_, const std::string address, std::string port);	// 构造函数
    ~LogicServerImpl();	// 析构函数

    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

// grpc对外接口
    // 获取玩家收藏
    grpc::Status Get_player_collection(grpc::ServerContext* context, const rpc_server::GetPlayerCollectionReq* req, rpc_server::GetPlayerCollectionRes* res) override;  // 登录
    // 更新玩家收藏
    grpc::Status Update_player_collection(grpc::ServerContext* context, const rpc_server::UpdatePlayerCollectionReq* req, rpc_server::UpdatePlayerCollectionRes* res) override;  // 登录
    // 获取玩家成就
    grpc::Status Get_player_achievements(grpc::ServerContext* context, const rpc_server::GetPlayerAchievementsReq* req, rpc_server::GetPlayerAchievementsRes* res) override;  // 登录
    // 更新玩家成就
    grpc::Status Update_player_achievements(grpc::ServerContext* context, const rpc_server::UpdatePlayerAchievementsReq* req, rpc_server::UpdatePlayerAchievementsRes* res) override;  // 登录
    // 获取玩家任务
    grpc::Status Get_player_tasks(grpc::ServerContext* context, const rpc_server::GetPlayerTasksReq* req, rpc_server::GetPlayerTasksRes* res) override;  // 登录
    // 更新玩家任务
    grpc::Status Update_player_tasks(grpc::ServerContext* context, const rpc_server::UpdatePlayerTasksReq* req, rpc_server::UpdatePlayerTasksRes* res) override;  // 登录
    // 添加物品
    grpc::Status Add_item(grpc::ServerContext* context, const rpc_server::AddItemReq* req, rpc_server::AddItemRes* res) override;  // 登录
    // 使用物品
    grpc::Status Use_item(grpc::ServerContext* context, const rpc_server::UseItemReq* req, rpc_server::UseItemRes* res) override;  // 登录
    // 保存对局结果
    grpc::Status Save_battle_result(grpc::ServerContext* context, const rpc_server::SaveBattleResultReq* req, rpc_server::SaveBattleResultRes* res) override;  // 登录

private:
    void Init_connection_pool();    // 初始化链接池
    void register_server(); // 注册服务器
    void unregister_server(); // 注销服务器

    std::future<void> add_async_task(std::function<void()> task); // 添加异步任务
    void Worker_thread();   // 执行线程的任务

    // 定时任务：
    void Update_connection_pool();  // 更新连接池
    void Send_heartbeat();  // 发送心跳包

// grpc工具函数
    void Handle_get_player_collection(const rpc_server::GetPlayerCollectionReq* req, rpc_server::GetPlayerCollectionRes* res); // 获取玩家收藏
    void Handle_update_player_collection(const rpc_server::UpdatePlayerCollectionReq* req, rpc_server::UpdatePlayerCollectionRes* res); // 更新玩家收藏
    void Handle_get_player_achievements(const rpc_server::GetPlayerAchievementsReq* req, rpc_server::GetPlayerAchievementsRes* res); // 获取玩家成就
    void Handle_update_player_achievements(const rpc_server::UpdatePlayerAchievementsReq* req, rpc_server::UpdatePlayerAchievementsRes* res); // 更新玩家成就
    void Handle_get_player_tasks(const rpc_server::GetPlayerTasksReq* req, rpc_server::GetPlayerTasksRes* res); // 获取玩家任务
    void Handle_update_player_tasks(const rpc_server::UpdatePlayerTasksReq* req, rpc_server::UpdatePlayerTasksRes* res); // 更新玩家任务
    void Handle_add_item(const rpc_server::AddItemReq* req, rpc_server::AddItemRes* res); // 添加物品
    void Handle_use_item(const rpc_server::UseItemReq* req, rpc_server::UseItemRes* res); // 使用物品
    void Handle_save_battle_result(const rpc_server::SaveBattleResultReq* req, rpc_server::SaveBattleResultRes* res); // 保存对局结果

private:
    std::string server_address; // 服务器地址
    std::string server_port;    // 服务器端口
    rpc_server::ServerType server_type;  // 服务器类型

    LoggerManager& logger_manager;  // 日志管理器
    RedisClient redis_client;    // Redis客户端
    AchievementManager achievement_manager; // 成就管理器

    std::unique_ptr<rpc_server::CentralServer::Stub> central_stub;	// 中心服务存根
    ConnectionPool db_connection_pool;   // 数据库服务器连接池

    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;    // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 停止线程标志
};

#endif // LOGIC_SERVER_H
