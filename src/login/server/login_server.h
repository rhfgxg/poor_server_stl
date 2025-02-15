#ifndef LOGIN_SERVER_H
#define LOGIN_SERVER_H

#include "common.grpc.pb.h" // 公共模块：包含公共数据类型，枚举
#include "consts/const_log.h"  // 日志类型
#include "server_login.grpc.pb.h"	// 登录服务
#include "server_db.grpc.pb.h"    // 数据库服务
#include "server_central.grpc.pb.h"	// 中心服务
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
// token
#include <jwt-cpp/jwt.h>    // token生成与解析
#include <chrono>   // 时间

// 登录服务实现类
class LoginServerImpl final : public rpc_server::LoginServer::Service
{
public:
    LoginServerImpl(LoggerManager& logger_manager_, const std::string address, std::string port);	// 构造函数
    ~LoginServerImpl();	// 析构函数

    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

// grpc服务接口
    //登录
    grpc::Status Login(grpc::ServerContext* context, const rpc_server::LoginReq* req, rpc_server::LoginRes* res) override;
    // 登出
    grpc::Status Logout(grpc::ServerContext* context, const rpc_server::LogoutReq* req, rpc_server::LogoutRes* res) override;
    // 注册
    grpc::Status Register(grpc::ServerContext* context, const rpc_server::RegisterReq* req, rpc_server::RegisterRes* res) override;
    // token校验
    grpc::Status Authenticate(grpc::ServerContext* context, const rpc_server::AuthenticateReq* req, rpc_server::AuthenticateRes* res) override;
    // 修改密码服务
    grpc::Status Change_password(grpc::ServerContext* context, const rpc_server::ChangePasswordReq* req, rpc_server::ChangePasswordRes* res) override;
    // 检查用户是否在线
    grpc::Status Is_user_online(grpc::ServerContext* context, const rpc_server::IsUserOnlineReq* req, rpc_server::IsUserOnlineRes* res) override;

private:
    // 初始化
    void register_server(); // 注册服务器
    void unregister_server(); // 注销服务器
    void Init_connection_pool();    // 初始化链接池

    std::future<void> add_async_task(std::function<void()> task); // 添加异步任务
    void Worker_thread();   // 执行线程的任务

    void Handle_login(const rpc_server::LoginReq* req,rpc_server::LoginRes* res);    // 登录
    void Handle_logout(const rpc_server::LogoutReq* req, rpc_server::LogoutRes* res);    // 登出
    void Handle_register(const rpc_server::RegisterReq* req,rpc_server::RegisterRes* res);    // 注册
    void Handle_authenticate(const rpc_server::AuthenticateReq* req,rpc_server::AuthenticateRes* res);    // 令牌验证
    void Handle_change_password(const rpc_server::ChangePasswordReq* req, rpc_server::ChangePasswordRes* res);    // 修改密码
    void Handle_is_user_online(const rpc_server::IsUserOnlineReq* req, rpc_server::IsUserOnlineRes* res);    // 获取在线用户列表

    // 定时任务：
    void Update_connection_pool();  // 更新连接池
    void Send_heartbeat();  // 发送心跳包

    // 工具函数
    std::string GenerateToken(const std::string& account);    // 生成用户 token
    bool ValidateToken(const std::string& token, const std::string& account);  // 验证 token

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

#endif // LOGIN_SERVER_H
