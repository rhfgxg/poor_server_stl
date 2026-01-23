#ifndef REDIS_CLUSTER_CLIENT_H
#define REDIS_CLUSTER_CLIENT_H

// ============================================================================
// 注意：此文件暂未使用
// 当前架构使用 本地 Redis + MySQL 实现数据存储
// 后续如需支持 Redis 集群，可重新启用此模块
// ============================================================================

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <chrono>
#include <unordered_map>
#include <shared_mutex>
#include <cpp_redis/cpp_redis>

// Redis 集群客户端
// 支持 Redis Cluster 模式，自动路由到正确的节点
class RedisClusterClient
{
public:
    RedisClusterClient();
    ~RedisClusterClient();


    // 连接到 Redis 集群
    // @param nodes 集群节点列表，格式："host:port"
    bool connect(const std::vector<std::string>& nodes);
    
    // 断开连接
    void disconnect();
    
    // 检查连接状态
    bool is_connected() const;

    // ==================== 基本命令 ====================
    
    // SET 命令
    bool set(const std::string& key, const std::string& value);
    
    // SET 命令（带过期时间）
    bool set_with_expire(const std::string& key, const std::string& value, std::chrono::seconds ttl);
    
    // GET 命令
    std::optional<std::string> get(const std::string& key);
    
    // DEL 命令
    bool del(const std::string& key);
    
    // EXISTS 命令
    bool exists(const std::string& key);
    
    // EXPIRE 命令
    bool expire(const std::string& key, std::chrono::seconds ttl);

    // ==================== Hash 命令 ====================
    
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    std::optional<std::string> hget(const std::string& key, const std::string& field);
    bool hdel(const std::string& key, const std::string& field);
    std::unordered_map<std::string, std::string> hgetall(const std::string& key);

    // ==================== List 命令 ====================
    
    bool lpush(const std::string& key, const std::string& value);
    bool rpush(const std::string& key, const std::string& value);
    std::optional<std::string> lpop(const std::string& key);
    std::optional<std::string> rpop(const std::string& key);
    std::vector<std::string> lrange(const std::string& key, int start, int stop);

    // ==================== Set 命令 ====================
    
    bool sadd(const std::string& key, const std::string& member);
    bool srem(const std::string& key, const std::string& member);
    bool sismember(const std::string& key, const std::string& member);
    std::vector<std::string> smembers(const std::string& key);

    // ==================== Sorted Set 命令 ====================
    
    bool zadd(const std::string& key, double score, const std::string& member);
    bool zrem(const std::string& key, const std::string& member);
    std::vector<std::pair<std::string, double>> zrange(const std::string& key, int start, int stop);
    std::vector<std::pair<std::string, double>> zrevrange(const std::string& key, int start, int stop);

private:
    // 计算 key 的 slot
    uint16_t calculate_slot(const std::string& key) const;
    
    // 根据 slot 获取节点客户端
    std::shared_ptr<cpp_redis::client> get_client_for_key(const std::string& key);
    
    // 刷新集群拓扑
    bool refresh_cluster_topology();
    
    // 解析节点地址
    bool parse_node_address(const std::string& node_str, std::string& host, size_t& port);

private:
    // 集群节点信息
    struct ClusterNode
    {
        std::string id;
        std::string host;
        size_t port;
        std::vector<std::pair<uint16_t, uint16_t>> slots;  // slot 范围
        std::shared_ptr<cpp_redis::client> client;
    };
    
    std::vector<ClusterNode> nodes_;
    mutable std::shared_mutex nodes_mutex_;
    
    // Slot 到节点的映射
    std::array<ClusterNode*, 16384> slot_map_;
    
    bool connected_;
};

#endif // REDIS_CLUSTER_CLIENT_H
