#ifndef GATEWAY_SERVER_H
#define GATEWAY_SERVER_H

#include "common.grpc.pb.h" // 公共模块：包含公共数据类型，枚举
#include "consts/const_log.h"  // 日志类型
#include "server_gateway.grpc.pb.h"
#include "server_central.grpc.pb.h"
#include "server_login.grpc.pb.h"
#include "server_file.grpc.pb.h"
#include "connection_pool.h"    // 连接池
#include "logger_manager.h"     // 日志管理器
#include "redis_client.h"       // Redis客户端

#include <grpcpp/grpcpp.h>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <lua.hpp>

// 网关服务器对外接口
class GatewayServerImpl final: public rpc_server::GatewayServer::Service
{
public:
    GatewayServerImpl(LoggerManager& logger_manager_, const std::string address, const std::string port);
    ~GatewayServerImpl();

    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

// grpc对外接口
    // 客户端注册
    grpc::Status Client_register(grpc::ServerContext* context, const rpc_server::ClientRegisterReq* req, rpc_server::ClientRegisterRes* res);
    // 接收客户端心跳
    grpc::Status Client_heartbeat(grpc::ServerContext* context, const rpc_server::ClientHeartbeatReq* req, rpc_server::ClientHeartbeatRes* res);
    // 发送网关服务器连接池
    grpc::Status Get_Gateway_pool(grpc::ServerContext* context, const rpc_server::GetGatewayPoolReq* req, rpc_server::GetGatewayPoolRes* res);
    // 转发服务请求
    grpc::Status Request_forward(grpc::ServerContext* context, const rpc_server::ForwardReq* req, rpc_server::ForwardRes* res);

private:
    // 初始化
    void register_server(); // 注册服务器
    void unregister_server(); // 注销服务器
    void Init_connection_pool();    // 初始化链接池

    // 多线程
    std::future<void> add_async_task(std::function<void()> task); // 添加异步任务
    void Worker_thread();   // 执行线程的任务

    // 发送网关服务器连接池
    grpc::Status Handle_return_gateway_poor(const rpc_server::GetGatewayPoolReq* req, rpc_server::GetGatewayPoolRes* res);
// 处理转发请求
    // 登录服务器
    grpc::Status Forward_to_login_service(const std::string& payload, rpc_server::ForwardRes* res); // 登录
    grpc::Status Forward_to_logout_service(const std::string& payload, rpc_server::ForwardRes* res);    // 登出
    grpc::Status Forward_to_register_service(const std::string& payload, rpc_server::ForwardRes* res);  // 注册
    // 文件服务器
    grpc::Status Forward_to_file_transmission_ready_service(const std::string& payload, rpc_server::ForwardRes* res); // 文件传输准备
    grpc::Status Forward_to_file_delete_service(const std::string& payload, rpc_server::ForwardRes* res);   // 文件删除
    // 定时任务：
    void Update_connection_pool();  // 更新连接池
    void Send_heartbeat();  // 发送心跳包

private:
    // 服务器信息
    std::string server_address; // 服务器地址
    std::string server_port;    // 服务器端口
    rpc_server::ServerType server_type;  // 服务器类型

    LoggerManager& logger_manager;  // 日志管理器
    RedisClient redis_client;    // Redis客户端

    std::unique_ptr<rpc_server::CentralServer::Stub> central_stub;  // 中心服务器的服务存根
    ConnectionPool login_connection_pool;   // 登录服务器连接池
    ConnectionPool file_connection_pool;    // 文件服务器连接池
    ConnectionPool gateway_connection_pool;    // 网关服务器连接池

    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;    // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 停止线程标志
};

#endif // GATEWAY_SERVER_H
