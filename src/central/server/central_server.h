#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H

#include "common.grpc.pb.h" // 公共模块：包含公共数据类型，枚举
#include "consts/const_log.h"  // 日志类型
#include "server_central.grpc.pb.h" // 中心服务器
#include "server_db.grpc.pb.h"    // 数据库服务器
#include "server_file.grpc.pb.h"    // 文件服务器
#include "server_gateway.grpc.pb.h" // 网关服务器
#include "server_login.grpc.pb.h"   // 登录服务器
#include "server_logic.grpc.pb.h"   // 逻辑服务器
#include "connection_pool.h"    // 连接池
#include "logger_manager.h"     // 日志管理器

#include <grpcpp/grpcpp.h>
#include <thread>   // 线程
#include <chrono>   // 时间
#include <vector>
#include <queue>
#include <mutex>    // 互斥锁
#include <condition_variable>   // 条件变量
#include <future>
#include <unordered_map> // 哈希表
#include <lua.hpp>  // lua

// 心跳记录
struct HeartbeatRecord
{
    rpc_server::ServerType server_type = rpc_server::ServerType::UNKNOWN;  // 服务器类型
    std::string address;    // 服务器地址
    std::string port;   // 服务器端口
    std::chrono::steady_clock::time_point last_heartbeat;   // 最后一次心跳时间
};

// 中心服务器实现类
class CentralServerImpl final: public rpc_server::CentralServer::Service
{
public:
    CentralServerImpl(LoggerManager& logger_manager_, const std::string address, const std::string port);
    ~CentralServerImpl();

// grpc对外接口
    // 服务器注册
    grpc::Status Register_server(grpc::ServerContext* context, const rpc_server::RegisterServerReq* req, rpc_server::RegisterServerRes* res);
    // 服务器断开
    grpc::Status Unregister_server(grpc::ServerContext* context, const rpc_server::UnregisterServerReq* req, rpc_server::UnregisterServerRes* res);
    // 获取连接池中所有链接
    grpc::Status Get_connec_poor(grpc::ServerContext* context, const rpc_server::MultipleConnectPoorReq* req, rpc_server::MultipleConnectPoorRes* res);
    // 接收心跳包
    grpc::Status Heartbeat(grpc::ServerContext* context, const rpc_server::HeartbeatReq* req, rpc_server::HeartbeatRes* res);

// 线程池
    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

private:    // 私有函数
    // 初始化
    std::future<void> add_async_task(std::function<void()> task); // 添加异步任务
    void Worker_thread();   // 线程池工作函数

    // grpc工具函数
    // 执行注册服务器
    void Handle_register_server(const rpc_server::RegisterServerReq* req, rpc_server::RegisterServerRes* res);
    // 释放连接池中服务器连接
    void Release_server_connection(rpc_server::ServerType server_type, const std::string& address, const std::string& port);
    // 返回连接池中的连接
    void All_connec_poor(const rpc_server::MultipleConnectPoorReq* req, rpc_server::MultipleConnectPoorRes* res);

    // 定时任务
    // 定时检查心跳记录
    void check_heartbeat();

private:    // 私有成员
    std::string server_address; // 服务器地址
    std::string server_port;    // 服务器端口
    rpc_server::ServerType server_type;  // 服务器类型

    LoggerManager& logger_manager;  // 日志管理器
    
    std::unordered_map<std::string, HeartbeatRecord> heartbeat_records;  // 心跳记录
    std::mutex heartbeat_mutex; // 心跳记录互斥锁

    // 连接池
    ConnectionPool battle_connection_pool;
    ConnectionPool central_connection_pool;
    ConnectionPool db_connection_pool;
    ConnectionPool gateway_connection_pool;
    ConnectionPool logic_connection_pool;
    ConnectionPool login_connection_pool;
    ConnectionPool file_connection_pool;

    // 线程池
    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;   // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 线程停止标志
};

#endif // CENTRAL_SERVER_H
