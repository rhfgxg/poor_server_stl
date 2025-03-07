#include "thread_manager.h"

ThreadManager::ThreadManager(size_t num_threads):
    stop_threads(false)
{
    workers.reserve(num_threads);   // 预留空间
}

ThreadManager::~ThreadManager()
{
    this->stop();
}

// 启动线程池
void ThreadManager::start()
{
    for(size_t i = 0; i < this->workers.capacity(); ++i)  // 遍历线程池
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

// 添加任务到任务队列
template<class F, class... Args>    // 模板函数（参数：类型F，可变类型 Args）
auto ThreadManager::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>    // 返回一个 std::future 对象，表示添加任务的结果
{
    using return_type = typename std::result_of<F(Args...)>::type;  // 函数的返回类型 return_type

    // std::make_shared<std::packaged_task<return_type()>>：创建一个 std::packaged_task 对象，并将其包装在 std::shared_ptr 中。std::packaged_task 是一个可以存储任务并在将来执行的对象。
    // std::bind(std::forward<F>(f), std::forward<Args>(args)...)：使用 std::bind 将任务函数 f 和参数 args 绑定在一起，并使用 std::forward 完美转发参数
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();  // 获取与 std::packaged_task 关联的 std::future 对象，以便可以在将来获取任务的结果

    {
        std::unique_lock<std::mutex> lock(this->queue_mutex);   // 使用 unique_lock 加锁
        if(this->stop_threads)  // 如果线程停止：抛出错误
        {
            throw std::runtime_error("enqueue on stopped ThreadManager");
        }

        this->tasks.emplace([task]() {  // 将任务（lambda函数）加入任务队列
            (*task)();
        });
    }

    this->condition.notify_one();   // 通知一个等待在条件变量上的线程，有新任务可以执行

    return res; // 返回任务结果
}
