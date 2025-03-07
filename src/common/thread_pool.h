#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>   // 线程
#include <chrono>   // 时间
#include <vector>
#include <queue>
#include <mutex>    // 互斥锁
#include <condition_variable>   // 条件变量
#include <future>
#include <unordered_map> // 哈希表

// 线程池管理器
class ThreadPool
{
public:
    ThreadPool(int num_threads);
    ~ThreadPool();
    void start(int num_threads);    // 启动线程池
    void stop();    // 停止线程池
    std::future<void> add_async_task(std::function<void()> task); // 添加异步任务

    void Worker_thread();   // 线程池工作函数

private:
    std::vector<std::thread> thread_pool;   // 线程池
    std::queue<std::function<void()>> task_queue;   // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable queue_cv;   // 任务队列条件变量
    bool stop_threads = false;  // 线程停止标志
};

#endif // !THREAD_POOL_H
