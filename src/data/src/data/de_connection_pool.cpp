#include "db_connection_pool.h"

DBConnectionPool::DBConnectionPool(const std::string& uri,size_t pool_size):
    uri(uri),
    pool_size(pool_size)
{
    for(size_t i = 0; i < pool_size; ++i)
    {
        pool.push(mysqlx::Session(uri));    // 创建数据库连接，并加入连接池
    }
}

DBConnectionPool::~DBConnectionPool()
{
    while(!pool.empty())
    {
        pool.front().close();
        pool.pop();
    }
}

// 获取数据库连接
mysqlx::Session DBConnectionPool::get_connection()
{
    std::unique_lock<std::mutex> lock(pool_mutex);  // 加锁
    pool_cv.wait(lock,[this] {
        return !pool.empty();
    });    // 等待连接池不为空

    mysqlx::Session session = std::move(pool.front());  // 获取数据库连接
    pool.pop(); // 弹出连接
    return session;
}

// 释放数据库连接
void DBConnectionPool::release_connection(mysqlx::Session session)
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // 加锁
    pool.push(std::move(session));  // 释放数据库连接
    pool_cv.notify_one();   // 通知有新的连接
}
