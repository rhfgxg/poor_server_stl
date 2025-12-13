#ifndef REDIS_CLIENT
#define REDIS_CLIENT

#include <cpp_redis/cpp_redis>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

// Windows 平台需要这些头文件来处理 WinSock
#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#endif

// Redis 管理器：封装 cpp_redis 客户端，提供配置加载与常用操作
class RedisClient
{
public:
    RedisClient();
    ~RedisClient();

    // 直接连接或通过配置文件连接
    bool connect(const std::string& host, std::size_t port, std::uint32_t db = 0, std::uint32_t timeout_ms = 2000);
    bool connect_from_config(const std::vector<std::string>& search_paths = {});
    bool is_connected() const;

    // 常用 Redis 操作（自动 sync_commit）
    bool set_with_expire(const std::string& key, const std::string& value, std::chrono::seconds ttl);
    std::optional<std::string> get_string(const std::string& key);
    bool delete_key(const std::string& key);
    bool key_exists(const std::string& key);
    bool refresh_ttl(const std::string& key, std::chrono::seconds ttl);

    // 仍然暴露底层客户端，供特殊场景使用
    std::shared_ptr<cpp_redis::client> get_client() const;

private:
    bool ensure_connection_locked();
    bool connect_locked();

private:
    mutable std::mutex client_mutex_;
    std::shared_ptr<cpp_redis::client> m_client;
    std::string host_;
    std::size_t port_{0};
    std::uint32_t db_index_{0};
    std::uint32_t timeout_ms_{2000};
};

#endif // !REDIS_CLIENT
