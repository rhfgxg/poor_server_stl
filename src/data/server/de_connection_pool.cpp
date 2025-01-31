#include "db_connection_pool.h"
#include <iostream>

DBConnectionPool::DBConnectionPool(const std::string& uri, size_t pool_size):
    uri(uri),   // 数据库连接地址
    pool_size(pool_size)    // 连接池大小
{
    for(size_t i = 0; i < pool_size; ++i)
    {
        try
        {
            pool.push(mysqlx::Session(uri));    // 创建数据库连接，并加入连接池
        }
        catch(const mysqlx::Error &err)
        {
            std::cerr << "Error: " << err.what() << std::endl;
        }
        catch(std::exception &ex)
        {
            std::cerr << "STD Exception: " << ex.what() << std::endl;
        }
        catch(...)
        {
            std::cerr << "Unknown exception caught" << std::endl;
        }
    }
}

DBConnectionPool::~DBConnectionPool()
{
    while(!this->pool.empty())
    {
        this->pool.front().close();
        this->pool.pop();
    }
}

// 获取数据库连接
mysqlx::Session DBConnectionPool::get_connection()
{
    std::unique_lock<std::mutex> lock(this->pool_mutex);  // 加锁
    pool_cv.wait(lock,[this] {
        return !this->pool.empty();
    });    // 等待连接池不为空

    mysqlx::Session session = std::move(this->pool.front());  // 获取数据库连接
    this->pool.pop(); // 弹出连接
    return session;
}

// 释放数据库连接
void DBConnectionPool::release_connection(mysqlx::Session session)
{
    std::lock_guard<std::mutex> lock(this->pool_mutex);   // 加锁
    this->pool.push(std::move(session));  // 释放数据库连接
    this->pool_cv.notify_one();   // 通知有新的连接
}
