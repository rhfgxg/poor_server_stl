#include "redis_client.h"

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

std::shared_ptr<cpp_redis::client> RedisClient::get_client() const
{
    return m_client;
}
