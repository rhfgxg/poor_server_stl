#include "connection_pool.h"
#include <chrono>

ConnectionPool::ConnectionPool(size_t pool_size_):
    pool_size(pool_size_)
{
}

ConnectionPool::~ConnectionPool()
{
    std::lock_guard<std::mutex> lock(pool_mutex);
    for(auto& pair : pool_map)
    {
        while(!pair.second.empty())
        {
            pair.second.pop();
        }
    }
}

/***************************************** 其他服务器使用连接池 ********************************************/
 // 获取链接
std::shared_ptr<grpc::Channel> ConnectionPool::get_connection(rpc_server::ServerType server_type)
{
    std::unique_lock<std::mutex> lock(pool_mutex);

    const auto has_connection = [this, server_type]() {
        auto it = this->pool_map.find(server_type);
        return it != this->pool_map.end() && !it->second.empty();
    };

    if (!has_connection())
    {
        if (!pool_cv.wait_for(lock, std::chrono::seconds(5), has_connection))
        {
            return nullptr;
        }
    }

    auto it = this->pool_map.find(server_type);
    if (it == this->pool_map.end() || it->second.empty())
    {
        return nullptr;
    }

    auto connection = it->second.front();
    it->second.pop();
    return connection;
}

// 释放链接
void ConnectionPool::release_connection(rpc_server::ServerType server_type, std::shared_ptr<grpc::Channel> connection)
{
    if (!connection)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(this->pool_mutex);
    auto it = this->pool_map.find(server_type);
    if (it == this->pool_map.end())
    {
        return;
    }

    it->second.push(std::move(connection));
    this->pool_cv.notify_one();
}

// 更新连接池中的连接
void ConnectionPool::update_connections(rpc_server::ServerType server_type,const std::string& server_address,const std::string& server_port)
{
    std::lock_guard<std::mutex> lock(this->pool_mutex);

    this->server_info_map[server_type] = {server_address,server_port};

    auto& queue = this->pool_map[server_type];
    while(!queue.empty())
    {
        queue.pop();
    }

    for(size_t i=0; i<pool_size; ++i)
    {
        auto channel = New_connection(server_address,server_port);
        if (channel)
        {
            queue.push(channel);
        }
    }

    this->pool_cv.notify_all();
}

/***************************************** 工具函数 ******************************************************/
// 向中心服务器获取最新连接
std::shared_ptr<grpc::Channel> ConnectionPool::New_connection(const std::string& server_address, const std::string& server_port)
{
    return grpc::CreateChannel(server_address + ":" + server_port, grpc::InsecureChannelCredentials());
}

/**************************************** 中心服务器管理连接池的接口 ****************************************
// 添加链接
void ConnectionPool::add_server(rpc_server::ServerType server_type,const std::string& server_address,const std::string& server_port)
{
    update_connections(server_type, server_address, server_port);
}

// 删除指定服务器的链接
void ConnectionPool::remove_server(rpc_server::ServerType server_type,const std::string& server_address,const std::string& server_port)
{
    (void)server_address;
    (void)server_port;

    std::lock_guard<std::mutex> lock(this->pool_mutex);
    this->server_info_map.erase(server_type);
    this->pool_map.erase(server_type);
    this->pool_cv.notify_all();
}

// 获取所有连接池的状态
std::unordered_map<rpc_server::ServerType, std::vector<std::pair<std::string, std::string>>> ConnectionPool::get_all_connections()
{
    std::lock_guard<std::mutex> lock(this->pool_mutex);

    std::unordered_map<rpc_server::ServerType, std::vector<std::pair<std::string, std::string>>> all_connections;

    for(const auto& pair : this->server_info_map)
    {
        all_connections[pair.first].push_back(pair.second);
    }

    return all_connections;
}
