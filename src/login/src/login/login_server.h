#ifndef LOGIN_SERVER_H
#define LOGIN_SERVER_H

#include "server_login.grpc.pb.h"	// 登录服务
#include "server_data.grpc.pb.h"    // 数据库服务
#include "server_central.grpc.pb.h"	// 中心服务
#include "connection_pool.h"    // 连接池
#include "logger_manager.h"     // 日志管理器

#include <grpcpp/grpcpp.h>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <lua.hpp>

// 登录服务实现类
class LoginServerImpl final : public rpc_server::LoginServer::Service
{
public:
	LoginServerImpl(LoggerManager& logger_manager_);	// 构造函数
    ~LoginServerImpl();	// 析构函数

	void register_server(); // 注册服务器
	void unregister_server(); // 注销服务器

    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

    grpc::Status Login(grpc::ServerContext* context, const rpc_server::LoginReq* req, rpc_server::LoginRes* res) override;  // 登录
    grpc::Status Register(grpc::ServerContext* context, const rpc_server::RegisterReq* req, rpc_server::RegisterRes* res) override;   // 注册
    grpc::Status Authenticate(grpc::ServerContext* context, const rpc_server::AuthenticateReq* req, rpc_server::AuthenticateRes* res) override;   // 令牌验证

private:
    void Read_server_config();   // 读取服务器配置文件，初始化服务器地址和端口

	void Init_connection_pool();    // 初始化链接池
    void Worker_thread();   // 执行线程的任务

	std::string Handle_login(const std::string& database, const std::string& table, std::map<std::string, std::string> query);    // 登录
	std::string Handle_register(const std::string& database, const std::string& table, std::map<std::string, std::string> data);    // 注册
	std::string Handle_authenticate(const std::string& token);    // 令牌验证

    // 定时任务：
    void Update_connection_pool();  // 更新连接池
    void Send_heartbeat();  // 发送心跳包

private:
    std::string server_address; // 服务器地址
    std::string server_port;    // 服务器端口
    rpc_server::ServerType server_type;  // 服务器类型

    // 日志管理器
    LoggerManager& logger_manager;

	std::unique_ptr<rpc_server::CentralServer::Stub> central_stub;	// 中心服务存根
	ConnectionPool db_connection_pool;   // 数据库服务器连接池

    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;    // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 停止线程标志
};

#endif // LOGIN_SERVER_H
