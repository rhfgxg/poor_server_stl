#ifndef DATA_SERVICE_H
#define DATA_SERVICE_H

#include "server_data.grpc.pb.h"
#include "server_central.grpc.pb.h"
#include "connection_pool.h"    // 连接池
#include "db_connection_pool.h" // 数据库连接池
#include "logger_manager.h"     // 日志管理器

#include <grpcpp/grpcpp.h>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <lua.hpp>

// 数据库服务实现类
class DatabaseServerImpl final: public rpc_server::DatabaseServer::Service
{
public:
    DatabaseServerImpl(LoggerManager& logger_manager_); // 参数：日志管理器，数据库连接池
    ~DatabaseServerImpl(); // 添加析构函数声明

    void register_server(); // 注册服务器
    void unregister_server(); // 注销服务器

    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

    // 服务实现
    // 添加数据库记录
    grpc::Status Create(grpc::ServerContext* context, const rpc_server::CreateReq* req, rpc_server::CreateRes* res) override;
    // 读取数据库记录
    grpc::Status Read(grpc::ServerContext* context, const rpc_server::ReadReq* req, rpc_server::ReadRes* res) override;
    // 更新数据库记录
    grpc::Status Update(grpc::ServerContext* context, const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res) override;
    // 删除数据库记录
    grpc::Status Delete(grpc::ServerContext* context, const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res) override;

private:
    void Read_server_config(lua_State* L, const std::string& file_url);   // 读取服务器配置文件，初始化服务器地址和端口
    std::string Read_db_config(lua_State* L, const std::string& file_url); // 读取 数据库配置配置文件，获得数据库连接字符串

    void Worker_thread();   // 线程池工作函数

    // 处理数据库操作的函数
    void Handle_create(const rpc_server::CreateReq* req, rpc_server::CreateRes* res);
    void Handle_read(const rpc_server::ReadReq* req, rpc_server::ReadRes* res);
    void Handle_update(const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res);
    void Handle_delete(const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res);

    // 定时任务：
    void Send_heartbeat();  // 发送心跳包

private:
    std::string server_address; // 服务器地址
    std::string server_port;    // 服务器端口
    rpc_server::ServerType server_type;  // 服务器类型

    LoggerManager& logger_manager;  // 日志管理器

    // 数据库连接池
    std::unique_ptr<DBConnectionPool> user_db_pool; // 用户数据库
    // 其他数据库连接池

    std::unique_ptr<rpc_server::CentralServer::Stub> central_stub;    // 中心服务存根

    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;    // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 停止线程标志
};

#endif // DATA_SERVICE_H
