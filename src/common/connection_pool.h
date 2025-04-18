#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include "common.grpc.pb.h" // 公共模块：包含公共数据类型，枚举

#include <grpcpp/grpcpp.h>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <condition_variable>

// 连接池类，管理所有连接
class ConnectionPool
{
public:
    ConnectionPool(size_t pool_size);   // 参数：连接池大小
    ~ConnectionPool();

    // 获取连接
    std::shared_ptr<grpc::Channel> get_connection(rpc_server::ServerType server_type);
    // 释放连接
    void release_connection(rpc_server::ServerType server_type, std::shared_ptr<grpc::Channel> connection);
    // 更新连接池中的连接
    void update_connections(rpc_server::ServerType server_type, const std::string& server_address, const std::string& server_port);

// 以下是中心服务器的接口
    // 向链接池中添加链接
    void add_server(rpc_server::ServerType server_type,const std::string& server_address,const std::string& server_port);
    // 删除指定服务器的链接
    void remove_server(rpc_server::ServerType server_type,const std::string& server_address,const std::string& server_port);
    // 获取所有连接池的状态
    std::unordered_map<rpc_server::ServerType, std::vector<std::pair<std::string, std::string>>> get_all_connections();

private:
    // 向中心服务器获取最新连接
    std::shared_ptr<grpc::Channel> New_connection(const std::string& server_address, const std::string& server_port);

private:
    size_t pool_size;  // 连接池大小
    std::unordered_map<rpc_server::ServerType, std::queue<std::shared_ptr<grpc::Channel>>> pool_map; // 连接池映射
    std::unordered_map<rpc_server::ServerType, std::pair<std::string,std::string>> server_info_map; // 服务器信息映射
    std::mutex pool_mutex; // 连接池互斥锁
    std::condition_variable pool_cv; // 连接池条件变量,直到连接池中有可用的连接
};

#endif // CONNECTION_POOL_H
