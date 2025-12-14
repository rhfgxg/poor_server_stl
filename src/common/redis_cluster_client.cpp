#include "redis_cluster_client.h"
#include <stdexcept>

// CRC16 查找表（Redis Cluster 使用的 CRC16-CCITT）
static const uint16_t crc16_tab[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

// CRC16 计算（用于 Redis Cluster slot）
static uint16_t crc16_ccitt(const char* buf, int len)
{
    uint16_t crc = 0;
    for (int i = 0; i < len; i++)
    {
        crc = (crc << 8) ^ crc16_tab[((crc >> 8) ^ *buf++) & 0x00FF];
    }
    return crc;
}

RedisClusterClient::RedisClusterClient()
    : connected_(false)
{
    slot_map_.fill(nullptr);
}

RedisClusterClient::~RedisClusterClient()
{
    disconnect();
}

bool RedisClusterClient::connect(const std::vector<std::string>& nodes)
{
    if (nodes.empty())
    {
        return false;
    }
    
    std::unique_lock lock(nodes_mutex_);
    
    // 连接到第一个可用节点
    for (const auto& node_str : nodes)
    {
        std::string host;
        size_t port;
        
        if (!parse_node_address(node_str, host, port))
        {
            continue;
        }
        
        auto client = std::make_shared<cpp_redis::client>();
        try
        {
            client->connect(host, port);
            
            ClusterNode node;
            node.host = host;
            node.port = port;
            node.client = client;
            
            nodes_.push_back(std::move(node));
            
            // 刷新集群拓扑
            connected_ = refresh_cluster_topology();
            return connected_;
        }
        catch (const std::exception&)
        {
            continue;
        }
    }
    
    return false;
}

void RedisClusterClient::disconnect()
{
    std::unique_lock lock(nodes_mutex_);
    
    for (auto& node : nodes_)
    {
        if (node.client && node.client->is_connected())
        {
            node.client->disconnect();
        }
    }
    
    nodes_.clear();
    slot_map_.fill(nullptr);
    connected_ = false;
}

bool RedisClusterClient::is_connected() const
{
    std::shared_lock lock(nodes_mutex_);
    return connected_;
}

// ==================== 基本命令 ====================

bool RedisClusterClient::set(const std::string& key, const std::string& value)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return false;
    }
    
    auto reply = client->set(key, value);
    client->sync_commit();
    
    return !reply.get().is_error();
}

bool RedisClusterClient::set_with_expire(const std::string& key, const std::string& value, std::chrono::seconds ttl)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return false;
    }
    
    auto reply = client->setex(key, static_cast<int>(ttl.count()), value);
    client->sync_commit();
    
    return !reply.get().is_error();
}

std::optional<std::string> RedisClusterClient::get(const std::string& key)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return std::nullopt;
    }
    
    auto reply = client->get(key);
    client->sync_commit();
    
    auto result = reply.get();
    if (result.is_null() || !result.is_string())
    {
        return std::nullopt;
    }
    
    return result.as_string();
}

bool RedisClusterClient::del(const std::string& key)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return false;
    }
    
    auto reply = client->del({key});
    client->sync_commit();
    
    return reply.get().is_integer() && reply.get().as_integer() > 0;
}

bool RedisClusterClient::exists(const std::string& key)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return false;
    }
    
    auto reply = client->exists({key});
    client->sync_commit();
    
    return reply.get().is_integer() && reply.get().as_integer() > 0;
}

bool RedisClusterClient::expire(const std::string& key, std::chrono::seconds ttl)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return false;
    }
    
    auto reply = client->expire(key, static_cast<int>(ttl.count()));
    client->sync_commit();
    
    return reply.get().is_integer() && reply.get().as_integer() == 1;
}

// ==================== Hash 命令 ====================

bool RedisClusterClient::hset(const std::string& key, const std::string& field, const std::string& value)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return false;
    }
    
    auto reply = client->hset(key, field, value);
    client->sync_commit();
    
    return !reply.get().is_error();
}

std::optional<std::string> RedisClusterClient::hget(const std::string& key, const std::string& field)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return std::nullopt;
    }
    
    auto reply = client->hget(key, field);
    client->sync_commit();
    
    auto result = reply.get();
    if (result.is_null() || !result.is_string())
    {
        return std::nullopt;
    }
    
    return result.as_string();
}

bool RedisClusterClient::hdel(const std::string& key, const std::string& field)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return false;
    }
    
    auto reply = client->hdel(key, {field});
    client->sync_commit();
    
    return reply.get().is_integer() && reply.get().as_integer() > 0;
}

std::unordered_map<std::string, std::string> RedisClusterClient::hgetall(const std::string& key)
{
    std::unordered_map<std::string, std::string> result;
    
    auto client = get_client_for_key(key);
    if (!client)
    {
        return result;
    }
    
    auto reply = client->hgetall(key);
    client->sync_commit();
    
    auto array = reply.get();
    if (array.is_array())
    {
        auto elements = array.as_array();
        for (size_t i = 0; i + 1 < elements.size(); i += 2)
        {
            if (elements[i].is_string() && elements[i + 1].is_string())
            {
                result[elements[i].as_string()] = elements[i + 1].as_string();
            }
        }
    }
    
    return result;
}

// ==================== Sorted Set 命令 ====================

bool RedisClusterClient::zadd(const std::string& key, double score, const std::string& member)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return false;
    }
    
    // cpp_redis API: zadd(key, options, score_members)
    // score_members 是一个 multimap<string, string>，其中 key=score, value=member
    std::multimap<std::string, std::string> score_members;
    score_members.insert({std::to_string(score), member});
    
    auto reply = client->zadd(key, {}, score_members);
    client->sync_commit();
    
    return !reply.get().is_error();
}

bool RedisClusterClient::zrem(const std::string& key, const std::string& member)
{
    auto client = get_client_for_key(key);
    if (!client)
    {
        return false;
    }
    
    auto reply = client->zrem(key, {member});
    client->sync_commit();
    
    return reply.get().is_integer() && reply.get().as_integer() > 0;
}

std::vector<std::pair<std::string, double>> RedisClusterClient::zrange(const std::string& key, int start, int stop)
{
    std::vector<std::pair<std::string, double>> result;
    
    auto client = get_client_for_key(key);
    if (!client)
    {
        return result;
    }
    
    auto reply = client->zrange(key, start, stop, true);  // with scores
    client->sync_commit();
    
    auto array = reply.get();
    if (array.is_array())
    {
        auto elements = array.as_array();
        for (size_t i = 0; i + 1 < elements.size(); i += 2)
        {
            if (elements[i].is_string() && elements[i + 1].is_string())
            {
                std::string member = elements[i].as_string();
                double score = std::stod(elements[i + 1].as_string());
                result.emplace_back(member, score);
            }
        }
    }
    
    return result;
}

std::vector<std::pair<std::string, double>> RedisClusterClient::zrevrange(const std::string& key, int start, int stop)
{
    std::vector<std::pair<std::string, double>> result;
    
    auto client = get_client_for_key(key);
    if (!client)
    {
        return result;
    }
    
    auto reply = client->zrevrange(key, start, stop, true);  // with scores
    client->sync_commit();
    
    auto array = reply.get();
    if (array.is_array())
    {
        auto elements = array.as_array();
        for (size_t i = 0; i + 1 < elements.size(); i += 2)
        {
            if (elements[i].is_string() && elements[i + 1].is_string())
            {
                std::string member = elements[i].as_string();
                double score = std::stod(elements[i + 1].as_string());
                result.emplace_back(member, score);
            }
        }
    }
    
    return result;
}

// ==================== 私有方法 ====================

uint16_t RedisClusterClient::calculate_slot(const std::string& key) const
{
    // 查找 hash tag {}
    size_t start = key.find('{');
    size_t end = key.find('}', start);
    
    std::string hash_key = key;
    if (start != std::string::npos && end != std::string::npos && end > start + 1)
    {
        // 使用 hash tag 中的内容
        hash_key = key.substr(start + 1, end - start - 1);
    }
    
    uint16_t crc = crc16_ccitt(hash_key.c_str(), hash_key.size());
    return crc % 16384;
}

std::shared_ptr<cpp_redis::client> RedisClusterClient::get_client_for_key(const std::string& key)
{
    uint16_t slot = calculate_slot(key);
    
    std::shared_lock lock(nodes_mutex_);
    
    if (slot_map_[slot])
    {
        return slot_map_[slot]->client;
    }
    
    // 如果没有找到，使用第一个节点（单机模式）
    if (!nodes_.empty())
    {
        return nodes_[0].client;
    }
    
    return nullptr;
}

bool RedisClusterClient::refresh_cluster_topology()
{
    // TODO: 使用 CLUSTER SLOTS 命令获取集群拓扑
    // 这里简化实现，假设单节点或不使用集群拓扑
    
    // 如果只有一个节点，所有 slot 都映射到它
    if (!nodes_.empty())
    {
        for (auto& slot : slot_map_)
        {
            slot = &nodes_[0];
        }
        return true;
    }
    
    return false;
}

bool RedisClusterClient::parse_node_address(const std::string& node_str, std::string& host, size_t& port)
{
    size_t colon_pos = node_str.find(':');
    if (colon_pos == std::string::npos)
    {
        return false;
    }
    
    host = node_str.substr(0, colon_pos);
    
    try
    {
        port = std::stoull(node_str.substr(colon_pos + 1));
        return true;
    }
    catch (...)
    {
        return false;
    }
}
