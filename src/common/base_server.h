#ifndef BASE_SERVER_H
#define BASE_SERVER_H

#include "common.grpc.pb.h"         // 公共模块：包含公共数据类型，枚举
#include "server_central.grpc.pb.h" // Central Server 协议
#include "connection_pool.h"        // 连接池
#include "logger_manager.h"         // 日志管理器
#include "thread_manager.h"         // 线程池管理器

#include <grpcpp/grpcpp.h>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

/**
 * @class BaseServer
 * @brief 服务器基类，封装所有服务器的通用功能
 * 
 */
class BaseServer
{
public:
    /**
     * @brief 构造函数
     * @param server_type 服务器类型
     * @param server_address 服务器地址
     * @param server_port 服务器端口
     * @param logger_manager 日志管理器引用
     * @param thread_count 线程池线程数（默认为硬件并发数）
     */
    BaseServer(
        rpc_server::ServerType server_type,
        const std::string& server_address,
        const std::string& server_port,
        LoggerManager& logger_manager,
        size_t thread_count = 0
    );

    /**
     * @brief 虚析构函数
     */
    virtual ~BaseServer();

    // 禁止拷贝和赋值
    BaseServer(const BaseServer&) = delete;
    BaseServer& operator=(const BaseServer&) = delete;

    /**
     * @brief 启动服务器
     * @return true 启动成功，false 启动失败
     */
    bool start();

    /**
     * @brief 停止服务器
     */
    void stop();

    /**
     * @brief 检查服务器是否运行中
     * @return true 运行中，false 已停止
     */
    bool is_running() const { return running_.load(); }

    /**
     * @brief 获取服务器类型
     */
    rpc_server::ServerType get_server_type() const { return server_type_; }

    /**
     * @brief 获取服务器地址
     */
    const std::string& get_server_address() const { return server_address_; }

    /**
     * @brief 获取服务器端口
     */
    const std::string& get_server_port() const { return server_port_; }

protected:
    // ==================== 钩子方法（子类可重写） ====================

    /**
     * @brief 服务器启动时调用（在线程池启动之后，注册到 Central 之前）
     * @return true 初始化成功，false 初始化失败
     */
    virtual bool on_start() { return true; }

    /**
     * @brief 服务器停止时调用（在注销和停止线程池之前）
     */
    virtual void on_stop() {}

    /**
     * @brief 心跳发送后调用
     * @param success 是否成功发送心跳
     */
    virtual void on_heartbeat_sent(bool success [[maybe_unused]]) {}

    /**
     * @brief 注册到 Central Server 后调用
     * @param success 是否成功注册
     */
    virtual void on_registered(bool success [[maybe_unused]]) {}

    /**
     * @brief 从 Central Server 注销后调用
     * @param success 是否成功注销
     */
    virtual void on_unregistered(bool success [[maybe_unused]]) {}

    // ==================== 工具方法（供子类使用） ====================

    /**
     * @brief 提交异步任务到线程池
     * @param f 函数对象
     * @param args 函数参数
     * @return std::future 用于获取任务结果
     */
    template<class F, class... Args>
    auto submit_task(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        if (thread_manager_)
        {
            return thread_manager_->enqueue(std::forward<F>(f), std::forward<Args>(args)...);
        }
        
        // 如果线程池未初始化，返回空 future
        using return_type = typename std::invoke_result<F, Args...>::type;
        std::promise<return_type> promise;
        promise.set_exception(std::make_exception_ptr(std::runtime_error("Thread pool not initialized")));
        return promise.get_future();
    }

    /**
     * @brief 提交无返回值的异步任务
     * @param task 任务函数
     */
    void submit_task_void(std::function<void()> task);

    /**
     * @brief 获取日志器
     * @param category 日志类别
     * @return 日志器指针
     */
    std::shared_ptr<spdlog::logger> get_logger(poor::LogCategory category);

    /**
     * @brief 记录启动日志
     * @param message 日志消息
     */
    void log_startup(const std::string& message);

    /**
     * @brief 记录关闭日志
     * @param message 日志消息
     */
    void log_shutdown(const std::string& message);

    /**
     * @brief 记录心跳日志
     * @param message 日志消息
     */
    void log_heartbeat(const std::string& message);

    /**
     * @brief 记录应用活动日志
     * @param message 日志消息
     */
    void log_activity(const std::string& message);

protected:
    // ==================== 成员变量 ====================
    
    // 服务器基本信息
    rpc_server::ServerType server_type_;    // 服务器类型
    std::string server_address_;            // 服务器地址
    std::string server_port_;               // 服务器端口
    std::atomic<bool> running_;             // 运行状态标志

    // 管理器
    LoggerManager& logger_manager_;         // 日志管理器
    std::unique_ptr<ThreadManager> thread_manager_;  // 线程池管理器

    // Central Server 连接
    std::string central_server_address_;    // Central Server 地址
    std::string central_server_port_;       // Central Server 端口
    std::unique_ptr<ConnectionPool> central_connection_pool_;  // 到 Central Server 的连接池

private:
    // ==================== 私有方法 ====================

    /**
     * @brief 初始化线程池
     * @param thread_count 线程数
     * @return true 成功，false 失败
     */
    bool init_thread_pool(size_t thread_count);

    /**
     * @brief 初始化 Central Server 连接
     * @return true 成功，false 失败
     */
    bool init_central_connection();

    /**
     * @brief 注册到 Central Server
     * @return true 成功，false 失败
     */
    bool register_to_central();

    /**
     * @brief 从 Central Server 注销
     * @return true 成功，false 失败
     */
    bool unregister_from_central();

    /**
     * @brief 启动心跳线程
     */
    void start_heartbeat();

    /**
     * @brief 停止心跳线程
     */
    void stop_heartbeat();

    /**
     * @brief 心跳线程函数
     */
    void heartbeat_thread_func();

    /**
     * @brief 发送心跳到 Central Server
     * @return true 成功，false 失败
     */
    bool send_heartbeat();

    /**
     * @brief 读取配置文件获取 Central Server 地址
     * @param config_file 配置文件路径
     * @return true 成功，false 失败
     */
    bool read_central_config(const std::string& config_file);

private:
    // 心跳相关
    std::thread heartbeat_thread_;          // 心跳线程
    std::atomic<bool> heartbeat_running_;   // 心跳线程运行标志
    std::chrono::seconds heartbeat_interval_;  // 心跳间隔（默认 30 秒）
};

#endif // BASE_SERVER_H
