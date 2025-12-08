#include "thread_manager.h"
#include <stdexcept>

ThreadManager::ThreadManager(size_t num_threads)
    : stop_threads(true), num_threads_(num_threads)
{
    if (num_threads_ == 0) {
        num_threads_ = std::thread::hardware_concurrency();
        if (num_threads_ == 0) num_threads_ = 4;
    }
    workers.reserve(num_threads_);   // 预留空间
}

ThreadManager::~ThreadManager()
{
    this->stop();
}

// 启动线程池
void ThreadManager::start()
{
    if (!workers.empty()) return; // 已启动

    stop_threads = false;
    for(size_t i = 0; i < num_threads_; ++i)
    {
        this->workers.emplace_back(&ThreadManager::worker_thread, this);  // 创建线程
    }
}

// 停止线程池
void ThreadManager::stop()
{
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex);   // 使用 unique_lock 加锁
        this->stop_threads = true;  // 设置线程停止标志为 true
    }
    this->condition.notify_all();   // 通知所有线程，检查 stop_threads 标志

    for(std::thread &worker : this->workers)    // 遍历线程池
    {
        if(worker.joinable())   // 如果线程可加入
        {
            worker.join();
        }
    }

    this->workers.clear();  // 清空线程池
    // 清空任务队列
    std::queue<std::function<void()>> empty;
    std::swap(this->tasks, empty);
}

// 线程池工作函数
void ThreadManager::worker_thread()
{
    while(true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);   // 使用 unique_lock 加锁

            this->condition.wait(lock, [this] { // 条件变量的检查条件
                return this->stop_threads || !this->tasks.empty();  // 任务队列不为空或停止标志为 true
            });

            if(this->stop_threads && this->tasks.empty())   // 如果停止标志为 true 且任务队列为空，退出线程
            {
                return;
            }
            task = std::move(this->tasks.front());  // 取出任务队列中的第一个任务
            this->tasks.pop();  // 移除任务队列中的第一个任务
        }
        task(); // 执行任务
    }
}

// 添加无返回值任务（简化版本）
void ThreadManager::enqueue_void(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex);
        if(this->stop_threads)
        {
            throw std::runtime_error("enqueue on stopped ThreadManager");
        }
        this->tasks.emplace(std::move(task));
    }
    this->condition.notify_one();
}

// 查询任务队列长度
size_t ThreadManager::task_count()
{
    std::unique_lock<std::mutex> lock(this->queue_mutex);
    return this->tasks.size();
}
