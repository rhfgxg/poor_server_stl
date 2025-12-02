#ifndef DB_SERVICE_H
#define DB_SERVICE_H

#include "common.grpc.pb.h"
#include "server_db.grpc.pb.h"
#include "base_server.h"
#include "db_connection_pool.h"
#include "logger_manager.h"

#include <grpcpp/grpcpp.h>
#include <memory>

// 前向声明 lua_State，避免在头文件中包含 lua.hpp
struct lua_State;

/**
 * @brief 数据库服务实现类（重构版本）
 * 
 * 继承自 BaseServer，自动获得：
 * - 线程池管理（ThreadManager）
 * - 服务器注册/注销
 * - 心跳发送
 * - 日志管理
 * 
 * 只需实现业务逻辑：
 * - 数据库操作（CRUD）
 * - 数据库连接池管理
 */
class DBServerImpl final : public BaseServer, public rpc_server::DBServer::Service
{
public:
    DBServerImpl(LoggerManager& logger_manager_, const std::string& address, const std::string& port);
    ~DBServerImpl() override;

    // gRPC 对外接口
    grpc::Status Create(grpc::ServerContext* context, const rpc_server::CreateReq* req, rpc_server::CreateRes* res) override;
    grpc::Status Read(grpc::ServerContext* context, const rpc_server::ReadReq* req, rpc_server::ReadRes* res) override;
    grpc::Status Update(grpc::ServerContext* context, const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res) override;
    grpc::Status Delete(grpc::ServerContext* context, const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res) override;
    grpc::Status Create_table(grpc::ServerContext* context, const rpc_server::CreateTableReq* req, rpc_server::CreateTableRes* res) override;

protected:
    // BaseServer 钩子方法
    bool on_start() override;
    void on_stop() override;

private:
    // 数据库操作处理函数
    void Handle_create(const rpc_server::CreateReq* req, rpc_server::CreateRes* res);
    void Handle_read(const rpc_server::ReadReq* req, rpc_server::ReadRes* res);
    void Handle_update(const rpc_server::UpdateReq* req, rpc_server::UpdateRes* res);
    void Handle_delete(const rpc_server::DeleteReq* req, rpc_server::DeleteRes* res);
    void Handle_create_table(const rpc_server::CreateTableReq* req, rpc_server::CreateTableRes* res);

    // 数据库连接池工具函数
    std::unique_ptr<mysqlx::Session> Get_connection(const std::string& db_name);
    void Release_connection(const std::string& db_name, std::unique_ptr<mysqlx::Session> session);

    // 配置读取
    bool init_db_pools();

private:
    // 数据库连接池
    std::unique_ptr<DBConnectionPool> poor_users_pool_;
    std::unique_ptr<DBConnectionPool> poor_file_pool_;
    std::unique_ptr<DBConnectionPool> hearthstone_pool_;
};

#endif // DB_SERVICE_H
