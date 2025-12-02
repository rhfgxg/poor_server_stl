#ifndef LOGIN_SERVER_H
#define LOGIN_SERVER_H

#include "common.grpc.pb.h"
#include "server_login.grpc.pb.h"
#include "server_db.grpc.pb.h"
#include "base_server.h"
#include "connection_pool.h"
#include "logger_manager.h"
#include "redis_client.h"

#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <grpcpp/grpcpp.h>
#include <jwt-cpp/jwt.h>
#include <chrono>
#include <memory>

/**
 * @brief 登录服务器实现类（重构版本）
 * 
 * 继承自 BaseServer，自动获得：
 * - 线程池管理（ThreadManager）
 * - 服务器注册/注销
 * - 心跳发送
 * - 日志管理
 * 
 * 只需实现业务逻辑：
 * - 用户登录/注册/登出
 * - Token 生成与验证
 * - 密码管理
 */
class LoginServerImpl final : public BaseServer, public rpc_server::LoginServer::Service
{
public:
    LoginServerImpl(LoggerManager& logger_manager_, const std::string address, std::string port);
    ~LoginServerImpl() override;

    // gRPC 服务接口
    grpc::Status Login(grpc::ServerContext* context, const rpc_server::LoginReq* req, rpc_server::LoginRes* res) override;
    grpc::Status Logout(grpc::ServerContext* context, const rpc_server::LogoutReq* req, rpc_server::LogoutRes* res) override;
    grpc::Status Register(grpc::ServerContext* context, const rpc_server::RegisterReq* req, rpc_server::RegisterRes* res) override;
    grpc::Status Authenticate(grpc::ServerContext* context, const rpc_server::AuthenticateReq* req, rpc_server::AuthenticateRes* res) override;
    grpc::Status Change_password(grpc::ServerContext* context, const rpc_server::ChangePasswordReq* req, rpc_server::ChangePasswordRes* res) override;
    grpc::Status Is_user_online(grpc::ServerContext* context, const rpc_server::IsUserOnlineReq* req, rpc_server::IsUserOnlineRes* res) override;

protected:
    // BaseServer 钩子方法
    bool on_start() override;
    void on_stop() override;
    void on_registered(bool success) override;

private:
    // 业务处理函数
    void Handle_login(const rpc_server::LoginReq* req, rpc_server::LoginRes* res);
    void Handle_logout(const rpc_server::LogoutReq* req, rpc_server::LogoutRes* res);
    void Handle_register(const rpc_server::RegisterReq* req, rpc_server::RegisterRes* res);
    void Handle_authenticate(const rpc_server::AuthenticateReq* req, rpc_server::AuthenticateRes* res);
    void Handle_change_password(const rpc_server::ChangePasswordReq* req, rpc_server::ChangePasswordRes* res);
    void Handle_is_user_online(const rpc_server::IsUserOnlineReq* req, rpc_server::IsUserOnlineRes* res);

    // 工具函数
    std::string Make_token(const std::string& account);
    bool Validate_token(const std::string& account_, const std::string& token_);
    std::string SHA256(const std::string& str);
    void Create_file_table(const std::string& account);
    
    // 连接池管理
    void Init_connection_pool();
    void Update_connection_pool();

private:
    RedisClient redis_client;
    ConnectionPool db_connection_pool;
    
    std::thread update_pool_thread_;
    std::atomic<bool> pool_update_running_{false};
};

#endif // LOGIN_SERVER_H
