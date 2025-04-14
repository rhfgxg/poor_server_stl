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
std::unique_ptr<mysqlx::Session> DBConnectionPool::Get_connection()
{
    std::unique_lock<std::mutex> lock(this->pool_mutex);
    pool_cv.wait(lock, [this] {
        return !this->pool.empty();
    });

    auto session = std::make_unique<mysqlx::Session>(std::move(this->pool.front()));
    this->pool.pop();
    return session;
}

// 释放数据库连接
void DBConnectionPool::Release_connection(std::unique_ptr<mysqlx::Session> session)
{
    std::lock_guard<std::mutex> lock(this->pool_mutex);
    this->pool.push(std::move(*session));
    this->pool_cv.notify_one();
}
