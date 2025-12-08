#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <type_traits>  // for std::invoke_result
#include <stdexcept>

// 线程池管理器
class ThreadManager
{
public:
    ThreadManager(size_t num_threads);
    ~ThreadManager();
    // 禁止拷贝和赋值
    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;

    // 添加任务到任务队列（有返回值）
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

    // 添加任务到任务队列（无返回值，简化版本）
    void enqueue_void(std::function<void()> task);

    // 查询任务队列长度
    size_t task_count();

    // 检查线程池是否运行中
    bool is_running() const { return !stop_threads; }

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
    size_t num_threads_; // 线程数
};

// 模板方法实现（必须在头文件中）
template<class F, class... Args>
auto ThreadManager::enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>
{
    using return_type = typename std::invoke_result<F, Args...>::type;

    // 使用 bind 生成可调用对象并放入 packaged_task 中
    auto bound_task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::move(bound_task));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // 不允许在停止后添加任务
        if (stop_threads) {
            throw std::runtime_error("enqueue on stopped ThreadManager");
        }

        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

#endif // !THREAD_MANAGER_H
