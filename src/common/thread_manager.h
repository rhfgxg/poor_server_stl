#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

// 线程池管理器
class ThreadManager
{
public:
    ThreadManager(size_t num_threads);
    ~ThreadManager();
    // 禁止拷贝和赋值
    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;

    // 添加任务到任务队列
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    void start();   // 启动线程池
    void stop();    // 停止线程池


private:
    // 线程池工作函数
    void worker_thread();

    std::vector<std::thread> workers;   // 线程池
    std::queue<std::function<void()>> tasks;   // 任务队列
    std::mutex queue_mutex; // 任务队列互斥锁
    std::condition_variable condition;   // 任务队列条件变量
    bool stop_threads;  // 线程停止标志
};

#endif // !THREAD_MANAGER_H
