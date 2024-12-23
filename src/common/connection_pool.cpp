#include "connection_pool.h"

ConnectionPool::ConnectionPool(size_t pool_size) :
    pool_size(pool_size)
{

}

ConnectionPool::~ConnectionPool() 
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // 加锁
    for (auto& pair : pool_map) // 遍历连接池
    {
        while (!pair.second.empty())    // 如果连接池不为空
        {
            pair.second.pop();  // 弹出连接
        }
    }
}

// 添加链接
void ConnectionPool::add_server(myproject::ServerType server_type, const std::string& server_address, const std::string& server_port)
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // 加锁
    server_info_map[server_type] = { server_address, server_port }; // 保存服务器信息
    for (size_t i = 0; i < pool_size; ++i)  // 遍历连接池，添加连接
    {
        auto channel = new_connection(server_address, server_port); // 创建连接
        pool_map[server_type].push(channel);    // 加入连接池
    }
}

// 删除指定服务器的链接
void ConnectionPool::remove_server(myproject::ServerType server_type, const std::string& server_address, const std::string& server_port)
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // 加锁
    server_info_map.erase(server_type); // 删除服务器信息
    while (!pool_map[server_type].empty())    // 如果连接池不为空
    {
        pool_map[server_type].pop();    // 弹出连接
    }
}

// 获取链接
std::shared_ptr<grpc::Channel> ConnectionPool::get_connection(myproject::ServerType server_type)
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // 加锁
    if (pool_map[server_type].empty())  // 如果连接池为空
    {
        auto& server_info = server_info_map[server_type];   // 获取服务器信息
        return new_connection(server_info.first, server_info.second);   // 创建新连接
    }
    auto connection = pool_map[server_type].front();    // 获取连接
    pool_map[server_type].pop();    // 弹出连接
    return connection;
}

// 释放链接
void ConnectionPool::release_connection(myproject::ServerType server_type, std::shared_ptr<grpc::Channel> connection)
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // 加锁
    pool_map[server_type].push(connection);  // 加入连接池
}

// 向中心服务器获取最新连接
std::shared_ptr<grpc::Channel> ConnectionPool::new_connection(const std::string& server_address, const std::string& server_port)
{
    return grpc::CreateChannel(server_address + ":" + server_port, grpc::InsecureChannelCredentials());
}

// 更新连接池中的连接
void ConnectionPool::update_connections(myproject::ServerType server_type, const std::string& server_address, const std::string& server_port)
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // 加锁

    // 更新服务器信息
    server_info_map[server_type] = { server_address, server_port };

    // 清空当前连接池
    while (!pool_map[server_type].empty()) {
        pool_map[server_type].pop();
    }

    // 添加新连接到连接池
    for (size_t i = 0; i < pool_size; ++i) {
        auto channel = new_connection(server_address, server_port);
        pool_map[server_type].push(channel);
    }
}

// 获取所有连接池的状态
std::unordered_map<myproject::ServerType, std::vector<std::pair<std::string, std::string>>> ConnectionPool::get_all_connections()
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // 加锁
    
    // 保存所有连接
    std::unordered_map<myproject::ServerType, std::vector<std::pair<std::string, std::string>>> all_connections;  
    
    for (const auto& pair : server_info_map)   // 遍历服务器信息
    {
        all_connections[pair.first].push_back(pair.second);    // 将服务器信息 加入容器
    }

    return all_connections;
}
