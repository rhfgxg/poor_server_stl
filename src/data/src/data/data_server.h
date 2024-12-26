#ifndef DATA_SERVICE_H
#define DATA_SERVICE_H

#include "server_data.grpc.pb.h"
#include "server_central.grpc.pb.h"
#include "connection_pool.h"    // 连接池
#include "logger_manager.h"     // 日志管理器

#include <grpcpp/grpcpp.h>
#include <mysqlx/xdevapi.h> // mysql
#include <thread>   // 线程
#include <chrono>   // 时间
#include <vector>
#include <queue>
#include <mutex>    // 互斥锁
#include <condition_variable>   // 条件变量

// 数据库连接池类
class DBConnectionPool
{
public:
    DBConnectionPool(const std::string& uri,size_t pool_size);
    ~DBConnectionPool();

    mysqlx::Session get_connection();
    void release_connection(mysqlx::Session session);

private:
    std::string uri;    // 数据库连接地址
    size_t pool_size;   // 连接池大小
    std::queue<mysqlx::Session> pool;   // 连接池
    std::mutex pool_mutex;  // 连接池互斥锁
    std::condition_variable pool_cv;    // 连接池条件变量
};

// 数据库服务实现类
class DatabaseServerImpl final: public myproject::DatabaseServer::Service
{
public:
    DatabaseServerImpl(LoggerManager& logger_manager_,DBConnectionPool& db_pool_); // 参数：日志管理器，数据库连接池
    ~DatabaseServerImpl(); // 添加析构函数声明

    void register_server(); // 注册服务器
    void unregister_server(); // 注销服务器

    void start_thread_pool(int num_threads);    // 启动线程池
    void stop_thread_pool();    // 停止线程池

    // 服务实现
    // 添加数据库记录
    grpc::Status Create(grpc::ServerContext* context,const myproject::CreateRequest* request,myproject::CreateResponse* response) override;
    // 读取数据库记录
    grpc::Status Read(grpc::ServerContext* context,const myproject::ReadRequest* request,myproject::ReadResponse* response) override;
    // 更新数据库记录
    grpc::Status Update(grpc::ServerContext* context,const myproject::UpdateRequest* request,myproject::UpdateResponse* response) override;
    // 删除数据库记录
    grpc::Status Delete(grpc::ServerContext* context,const myproject::DeleteRequest* request,myproject::DeleteResponse* response) override;

private:
    void worker_thread();   // 线程池工作函数

    // 处理数据库操作的函数
    void handle_create(const myproject::CreateRequest* request,myproject::CreateResponse* response);
    void handle_read(const myproject::ReadRequest* request,myproject::ReadResponse* response);
    void handle_update(const myproject::UpdateRequest* request,myproject::UpdateResponse* response);
    void handle_delete(const myproject::DeleteRequest* request,myproject::DeleteResponse* response);

    // 定时任务：
    void send_heartbeat();  // 发送心跳包

private:
    // 日志管理器
    LoggerManager& logger_manager;

    DBConnectionPool& db_pool; // 数据库连接池

    std::unique_ptr<myproject::CentralServer::Stub> central_stub;    // 中心服务存根

    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;    // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 停止线程标志
};

#endif // DATA_SERVICE_H
