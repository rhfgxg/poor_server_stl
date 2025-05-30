#ifndef BATTLE_SERVER_H
#define BATTLE_SERVER_H

#include "common.grpc.pb.h" // 公共模块：包含公共数据类型，枚举
#include "consts/const_log.h"  // 日志类型
#include "server_battle.grpc.pb.h"	// 战斗服务器
#include "server_central.grpc.pb.h"	// 中心服务器
#include "server_db.grpc.pb.h"    // 数据库服务器
#include "server_logic.grpc.pb.h"	// 逻辑服务器
#include "connection_pool.h"    // 连接池
#include "logger_manager.h"     // 日志管理器
#include "redis_client.h"       // Redis客户端

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

// 战斗服务器实现类
class BattleServerImpl final: public rpc_server::BattleServer::Service
{
public:
    BattleServerImpl(LoggerManager& logger_manager_, const std::string address, std::string port);	// 构造函数
    ~BattleServerImpl();	// 析构函数

    void register_server(); // 注册服务器
    void unregister_server(); // 注销服务器

    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

private:
    void Init_connection_pool();    // 初始化链接池

    std::future<void> add_async_task(std::function<void()> task); // 添加异步任务
    void Worker_thread();   // 执行线程的任务

    // 定时任务：
    void Update_connection_pool();  // 更新连接池
    void Send_heartbeat();  // 发送心跳包
private:
    std::string server_address; // 服务器地址
    std::string server_port;    // 服务器端口
    rpc_server::ServerType server_type;  // 服务器类型

    LoggerManager& logger_manager;  // 日志管理器
    RedisClient redis_client;    // Redis客户端

    std::unique_ptr<rpc_server::CentralServer::Stub> central_stub;	// 中心服务存根
    ConnectionPool db_connection_pool;   // 数据库服务器连接池

    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;    // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 停止线程标志
};

#endif // BATTLE_SERVER_H
