#ifndef REDIS_CLIENT
#define REDIS_CLIENT

#include <memory>  // std::shared_ptr
#include <cpp_redis/cpp_redis>

// Windows 平台需要这些头文件来处理 WinSock
#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#endif

// 工具类，封装redis链接，防止 windows 下的一个异常
class RedisClient
{
    /* redis 与 WINsock 冲突问题
     * redis 默认全局初始化了 WINsock，导致 grpc 无法正常工作
     * 使用该类封装 redis 客户端，避免 redis 初始化 WINsock
     * 由于 redis 客户端是异步的，所以使用 shared_ptr 管理 redis 客户端
     * 由于这是windows下的问题，所以只在windows下使用该类
     * 
     * 注意: Linux/WSL 环境下不需要手动初始化套接字，可以直接使用
     */
public:
    RedisClient();
    ~RedisClient();

    std::shared_ptr<cpp_redis::client> get_client() const;

private:
    std::shared_ptr<cpp_redis::client> m_client;
};

#endif // !REDIS_CLIENT
