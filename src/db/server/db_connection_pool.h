#ifndef DB_CONNECTION_POOL_H
#define DB_CONNECTION_POOL_H

#include <mysqlx/xdevapi.h> // mysql
#include <queue>
#include <mutex>

// 数据库连接池类
class DBConnectionPool
{
public:
    DBConnectionPool(const std::string& uri, size_t pool_size);
    ~DBConnectionPool();

    std::unique_ptr<mysqlx::Session> Get_connection();   // 获取数据库连接
    void Release_connection(std::unique_ptr<mysqlx::Session> session);   // 释放数据库连接

private:
    std::string uri;    // 数据库连接地址
    size_t pool_size;   // 连接池大小
    std::queue<mysqlx::Session> pool;   // 连接池
    std::mutex pool_mutex;  // 连接池互斥锁
    std::condition_variable pool_cv;    // 连接池条件变量
};

#endif // DB_CONNECTION_POOL_H
