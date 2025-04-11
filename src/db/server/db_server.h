#ifndef DB_SERVICE_H
#define DB_SERVICE_H

#include "common.grpc.pb.h" // 公共模块：包含公共数据类型，枚举
#include "server_db.grpc.pb.h"  // 数据库服务器
#include "server_central.grpc.pb.h" // 中心服务器
#include "consts/const_log.h"  // 日志类型
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
class DBServerImpl final: public rpc_server::DBServer::Service
{
public:
    DBServerImpl(LoggerManager& logger_manager_, const std::string address, const std::string port); // 参数：日志管理器，数据库连接池
    ~DBServerImpl(); // 添加析构函数声明

    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

// grpc对外接口
    // 添加数据
    grpc::Status Create(grpc::ServerContext* context, const rpc_server::CreateReq* req, rpc_server::CreateRes* res) override;
    // 查询数据
    grpc::Status Read(grpc::ServerContext* context, const rpc_server::ReadReq* req, rpc_server::ReadRes* res) override;
    // 更新数据
    grpc::Status Update(grpc::ServerContext* context, const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res) override;
    // 删除数据
    grpc::Status Delete(grpc::ServerContext* context, const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res) override;
    // 新建表
    grpc::Status Make_table(grpc::ServerContext* context, const rpc_server::MakeTableReq* req, rpc_server::MakeTableRes* res) override;
private:
    // 初始化
    std::string Read_db_config(lua_State* L, const std::string& file_url); // 读取 数据库配置配置文件，获得数据库连接字符串
    void register_server(); // 注册服务器
    void unregister_server(); // 注销服务器

    std::future<void> add_async_task(std::function<void()> task); // 添加异步任务
    void Worker_thread();   // 线程池工作函数

    // 处理数据库操作的函数
    void Handle_create(const rpc_server::CreateReq* req, rpc_server::CreateRes* res);   // 添加数据
    void Handle_read(const rpc_server::ReadReq* req, rpc_server::ReadRes* res); // 查询数据
    void Handle_update(const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res);   // 更新数据
    void Handle_delete(const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res);   // 删除数据
    void Handle_make_table(const rpc_server::MakeTableReq* req, rpc_server::MakeTableRes* res);   // 新建表

    // 定时任务：
    void Send_heartbeat();  // 发送心跳包

private:
    std::string server_address; // 服务器地址
    std::string server_port;    // 服务器端口
    rpc_server::ServerType server_type;  // 服务器类型

    LoggerManager& logger_manager;  // 日志管理器

    // 数据库连接池
    std::unique_ptr<DBConnectionPool> poor_users_pool; // 用户数据库
    std::unique_ptr<DBConnectionPool> poor_file_pool;  // 网盘数据库
    std::unique_ptr<DBConnectionPool> hearthstone_pool;   // 炉石传说数据库

    std::unique_ptr<rpc_server::CentralServer::Stub> central_stub;    // 中心服务存根

    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;    // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 停止线程标志
};

#endif // DB_SERVICE_H
