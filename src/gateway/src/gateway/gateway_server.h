#ifndef GATEWAY_SERVER_H
#define GATEWAY_SERVER_H

#include "server_gateway.grpc.pb.h"
#include "server_central.grpc.pb.h"
#include "server_login.grpc.pb.h"
#include "connection_pool.h"    // 连接池
#include "logger_manager.h"     // 日志管理器

#include <grpcpp/grpcpp.h>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

// 网关服务器对外接口
class GatewayServerImpl final: public myproject::GatewayServer::Service
{
public:
    GatewayServerImpl(LoggerManager& logger_manager_);
    ~GatewayServerImpl();

    void register_server(); // 注册服务器
    void unregister_server(); // 注销服务器

    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

    // 转发服务请求
    grpc::Status RequestForward(grpc::ServerContext* context,const myproject::ForwardRequest* request,myproject::ForwardResponse* response);

private:
    void init_connection_pool();    // 初始化链接池
    void worker_thread();   // 执行线程的任务

// 解析并转发负载
    // 登录服务器：登录服务
    grpc::Status forward_to_login_service(const std::string& payload,myproject::ForwardResponse* response);

private:
    LoggerManager& logger_manager;  // 日志管理器

    std::unique_ptr<myproject::CentralServer::Stub> central_stub;  // 中心服务器的服务存根
    ConnectionPool login_connection_pool;   // 登录服务器连接池
    
    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;    // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 停止线程标志
};

#endif // GATEWAY_SERVER_H
