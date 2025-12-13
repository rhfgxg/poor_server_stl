#include "redis_client.h"
#include "config_manager.h"

#include <array>
#include <stdexcept>
#include <vector>

namespace
{
    constexpr std::array<const char*, 5> DEFAULT_REDIS_CONFIG_PATHS = {
        "config/cfg_db.lua",
        "config/cpp/cfg_server/cfg_db.lua",
        "../config/cpp/cfg_server/cfg_db.lua",
        "../../config/cpp/cfg_server/cfg_db.lua",
        "../../../config/cpp/cfg_server/cfg_db.lua"
    };
}

RedisClient::RedisClient()
{
#ifdef _WIN32   // 下面代码使用了 Windows API：WSAStartup
    WORD version = MAKEWORD(2, 2);
    WSADATA data;
    if(WSAStartup(version, &data) != 0)
    {
        throw std::runtime_error("WSAStartup failed");
    }
#endif
    m_client = std::make_shared<cpp_redis::client>();
}

RedisClient::~RedisClient()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

bool RedisClient::connect(const std::string& host, std::size_t port, std::uint32_t db, std::uint32_t timeout_ms)
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    host_ = host;
    port_ = port;
    db_index_ = db;
    timeout_ms_ = timeout_ms;
    return connect_locked();
}

bool RedisClient::connect_from_config(const std::vector<std::string>& search_paths)
{
    ConfigManager config_manager;
    std::vector<std::string> paths = search_paths;
    paths.insert(paths.end(), DEFAULT_REDIS_CONFIG_PATHS.begin(), DEFAULT_REDIS_CONFIG_PATHS.end());

    if (!config_manager.load_from_paths(paths))
    {
        return false;
    }

    const std::string host = config_manager.get_string("redis.host").value_or("127.0.0.1");
    const auto port = static_cast<std::size_t>(config_manager.get_int("redis.port").value_or(6379));
    const auto db = static_cast<std::uint32_t>(config_manager.get_int("redis.db").value_or(0));

    return connect(host, port, db);
}

bool RedisClient::is_connected() const
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    return m_client && m_client->is_connected();
}

bool RedisClient::set_with_expire(const std::string& key, const std::string& value, std::chrono::seconds ttl)
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    if (!ensure_connection_locked())
    {
        return false;
    }

    auto set_reply = m_client->set(key, value);
    if (ttl.count() > 0)
    {
        m_client->expire(key, static_cast<int>(ttl.count()));
    }
    m_client->sync_commit();
    return !set_reply.get().is_error();
}

std::optional<std::string> RedisClient::get_string(const std::string& key)
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    if (!ensure_connection_locked())
    {
        return std::nullopt;
    }

    auto reply = m_client->get(key);
    m_client->sync_commit();
    auto result = reply.get();
    if (result.is_null() || !result.is_string())
    {
        return std::nullopt;
    }
    return result.as_string();
}

bool RedisClient::delete_key(const std::string& key)
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    if (!ensure_connection_locked())
    {
        return false;
    }

    auto reply = m_client->del({key});
    m_client->sync_commit();
    auto result = reply.get();
    return result.is_integer() && result.as_integer() > 0;
}

bool RedisClient::key_exists(const std::string& key)
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    if (!ensure_connection_locked())
    {
        return false;
    }

    auto reply = m_client->exists({key});
    m_client->sync_commit();
    auto result = reply.get();
    return result.is_integer() && result.as_integer() > 0;
}

bool RedisClient::refresh_ttl(const std::string& key, std::chrono::seconds ttl)
{
    if (ttl.count() <= 0)
    {
        return true;
    }

    std::lock_guard<std::mutex> lock(client_mutex_);
    if (!ensure_connection_locked())
    {
        return false;
    }

    auto reply = m_client->expire(key, static_cast<int>(ttl.count()));
    m_client->sync_commit();
    auto result = reply.get();
    return result.is_integer() && result.as_integer() == 1;
}

std::shared_ptr<cpp_redis::client> RedisClient::get_client() const
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    return m_client;
}

bool RedisClient::ensure_connection_locked()
{
    if (!m_client)
    {
        m_client = std::make_shared<cpp_redis::client>();
    }

    if (m_client->is_connected())
    {
        return true;
    }

    return connect_locked();
}

bool RedisClient::connect_locked()
{
    if (host_.empty() || port_ == 0)
    {
        return false;
    }

    if (!m_client)
    {
        m_client = std::make_shared<cpp_redis::client>();
    }

    if (m_client->is_connected())
    {
        m_client->disconnect();
    }

    try
    {
        m_client->connect(host_, port_, nullptr, timeout_ms_, 1, 1000);
    }
    catch (const std::exception&)
    {
        return false;
    }

    if (db_index_ != 0)
    {
        auto reply = m_client->select(db_index_);
        m_client->sync_commit();
        if (reply.get().is_error())
        {
            return false;
        }
    }

    return true;
}
