#ifndef CENTRAL_SERVER_H
#define CENTRAL_SERVER_H

#include "server_central.grpc.pb.h" // 中心服务器
#include "server_data.grpc.pb.h"    // 数据库服务器
#include "server_gateway.grpc.pb.h" // 网关服务器
#include "server_login.grpc.pb.h"   // 登录服务器
#include "connection_pool.h"    // 连接池
#include "logger_manager.h"     // 日志管理器

#include <grpcpp/grpcpp.h>
#include <thread>   // 线程
#include <chrono>   // 时间
#include <vector>
#include <queue>
#include <mutex>    // 互斥锁
#include <condition_variable>   // 条件变量
#include <unordered_map> // 哈希表

// 心跳记录
struct HeartbeatRecord
{
    myproject::ServerType server_type = myproject::ServerType::UNKNOWN;  // 服务器类型
    std::string address;    // 服务器地址
    std::string port;   // 服务器端口
    std::chrono::steady_clock::time_point last_heartbeat;   // 最后一次心跳时间
};

// 中心服务器实现类
class CentralServerImpl final: public myproject::CentralServer::Service
{
public:
    CentralServerImpl(LoggerManager& logger_manager_);  
    ~CentralServerImpl();

// grpc服务接口
    // 服务器注册
    grpc::Status RegisterServer(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response);
    // 服务器断开
    grpc::Status UnregisterServer(grpc::ServerContext* context, const myproject::UnregisterServerRequest* request, myproject::UnregisterServerResponse* response);
    // 获取连接池中所有链接
    grpc::Status GetConnectPoor(grpc::ServerContext* context, const myproject::ConnectPoorRequest* request, myproject::ConnectPoorResponse* response);
    // 接收心跳包
    grpc::Status Heartbeat(grpc::ServerContext* context, const myproject::HeartbeatRequest* request, myproject::HeartbeatResponse* response);

// 线程池
    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

// 定时任务
    // 定时检查心跳记录
    void check_heartbeat();

private:    // 私有函数
    void worker_thread();   // 线程池工作函数

    // 释放连接池中服务器连接
    void release_server_connection(myproject::ServerType server_type,const std::string& address,const std::string& port);

private:    // 私有成员
    LoggerManager& logger_manager;  // 日志管理器
    
    std::unordered_map<std::string, HeartbeatRecord> heartbeat_records;  // 心跳记录
    std::mutex heartbeat_mutex; // 心跳记录互斥锁

    // 连接池
    ConnectionPool central_connection_pool;
    ConnectionPool data_connection_pool;
    ConnectionPool gateway_connection_pool;
    ConnectionPool logic_connection_pool;
    ConnectionPool login_connection_pool;

    // 线程池
    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;   // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 线程停止标志
};

#endif // CENTRAL_SERVER_H
