#include "thread_pool.h"

ThreadPool::ThreadPool(int num_threads)
{
    start(num_threads);
}

ThreadPool::~ThreadPool()
{
    stop();
}

// 启动线程池
void start(int num_threads)
{
    for(int i=0; i<num_threads; ++i)
    {
        this->thread_pool.emplace_back(&CentralServerImpl::Worker_thread, this);   // 创建线程
    }
}

// 停止线程池
void stop()
{
    {
        std::lock_guard<std::mutex> lock(this->queue_mutex);  // 加锁
        this->stop_threads = true;    // 线程停止标志设置为 true，通知它应该停止工作
    }

    this->queue_cv.notify_all();  // 通知所有线程，检查 stop_threads 标志

    for(auto& thread : this->thread_pool) // 遍历线程池
    {
        if(thread.joinable())   // 如果线程可加入
        {
            thread.join();  // 加入线程
        }
    }
    this->thread_pool.clear();  // 清空线程池
    // 清空任务队列
    std::queue<std::function<void()>> empty;
    std::swap(this->task_queue, empty);
}

// 添加异步任务
std::future<void> add_async_task(std::function<void()> task)
{
    auto task_ptr = std::make_shared<std::packaged_task<void()>>(std::move(task));  // 创建任务
    std::future<void> task_future = task_ptr->get_future();    // 获取任务的 future 对象

    {
        std::lock_guard<std::mutex> lock(this->queue_mutex);  // 加锁
        this->task_queue.push([task_ptr]() {  // 将任务加入任务队列
            (*task_ptr)();
        });
    }
    this->queue_cv.notify_one();    // 通知线程池有新任务

    return task_future;
}

// 线程池工作函数
void Worker_thread()
{
    while(true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);  // 使用 unique_lock 加锁

            queue_cv.wait(lock, [this] {
                return !this->task_queue.empty() || (this->stop_threads);
            });  // 等待条件变量，直到任务队列不为空或停止标志为 true

            if(this->stop_threads && this->task_queue.empty())  // 如果停止标志为 true 且任务队列为空，退出线程
            {
                return;
            }

            task = std::move(this->task_queue.front());  // 取出任务队列中的第一个任务
            this->task_queue.pop();  // 移除任务队列中的第一个任务
        }
        task();  // 执行任务
    }
}
