#ifndef SKYNET_TCP_MANAGER_H
#define SKYNET_TCP_MANAGER_H

#include "logger_manager.h"
#include "connection_pool.h"
#include "server_central.grpc.pb.h"
#include "common.pb.h"              // rpc_server::ServiceType

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <set>
#include <mutex>
#include <functional>
#include <vector>
#include <condition_variable>

// 平台相关的 socket 定义
#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using SocketType = SOCKET;
    constexpr SocketType INVALID_SOCKET_VALUE = INVALID_SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using SocketType = int;
    constexpr SocketType INVALID_SOCKET_VALUE = -1;
    #define closesocket close
#endif

/**
 * @brief Skynet TCP 消息结构
 * 协议格式：[4字节长度(网络序)][2字节类型][Protobuf数据]
 * 长度字段包含类型和数据的总长度
 */
struct SkynetTcpMessage
{
    uint16_t msg_type{0};       // 消息类型（对应 rpc_server::ServiceType）
    std::string payload;        // 消息数据（Protobuf 序列化后的数据）
    
    SkynetTcpMessage() = default;
    SkynetTcpMessage(uint16_t type, const std::string& data) : msg_type(type), payload(data) {}
    SkynetTcpMessage(uint16_t type, std::string&& data) : msg_type(type), payload(std::move(data)) {}
    
    // 便捷构造：直接使用 ServiceType
    SkynetTcpMessage(rpc_server::ServiceType type, const std::string& data) 
        : msg_type(static_cast<uint16_t>(type)), payload(data) {}
    SkynetTcpMessage(rpc_server::ServiceType type, std::string&& data) 
        : msg_type(static_cast<uint16_t>(type)), payload(std::move(data)) {}
};

/**
* @brief Skynet TCP 管理器
 * 
 * 统一管理 Gateway 与 Skynet 之间的双向 TCP 通信：
 * - 服务器模式：接收来自 Skynet 的请求（如 DB 操作转发）
 * - 客户端模式：向 Skynet 发送请求（如游戏逻辑转发）
 * 
 * 共享统一的消息协议：[4字节长度][2字节类型][Protobuf数据]
 */
class SkynetTcpManager
{
public:
    // 回调类型定义
    using RequestHandler = std::function<std::string(uint16_t msg_type, const std::string& payload)>;
    using PushCallback = std::function<void(const SkynetTcpMessage&)>;
    
    /**
     * @brief 构造函数
     * @param logger_manager 日志管理器引用
     * @param central_pool 中心服务器连接池（用于获取 Skynet IP 白名单，可选）
     */
    explicit SkynetTcpManager(LoggerManager& logger_manager, ConnectionPool* central_pool = nullptr);
    
    ~SkynetTcpManager();
    
    // 禁止拷贝
    SkynetTcpManager(const SkynetTcpManager&) = delete;
    SkynetTcpManager& operator=(const SkynetTcpManager&) = delete;

    // ==================== 公共协议方法（静态） ====================
    
    /**
     * @brief 发送 TCP 消息
     * @param socket 目标 socket
     * @param msg 消息结构
     * @return true 发送成功
     */
    static bool send_message(SocketType socket, const SkynetTcpMessage& msg);
    
    /**
     * @brief 接收 TCP 消息
     * @param socket 源 socket
     * @param msg 接收的消息（输出参数）
     * @return true 接收成功
     */
    static bool receive_message(SocketType socket, SkynetTcpMessage& msg);

    // ==================== 服务器模式（接收 Skynet 请求） ====================
    
    /**
     * @brief 启动 TCP 服务器
     * @param port 监听端口
     * @return true 启动成功
     */
    bool start_server(int port = 8889);
    
    /**
     * @brief 停止 TCP 服务器
     */
    void stop_server();
    
    /**
     * @brief 检查服务器是否运行中
     */
    bool is_server_running() const { return server_running_.load(); }
    
    /**
     * @brief 设置服务器请求处理回调
     * @param handler 处理回调函数
     */
    void set_request_handler(RequestHandler handler) { request_handler_ = std::move(handler); }
    
    /**
     * @brief 更新 Skynet IP 白名单（从 Central 获取）
     */
    void update_whitelist();
    
    /**
     * @brief 手动添加 IP 到白名单
     */
    void add_to_whitelist(const std::string& ip);
    
    /**
     * @brief 检查 IP 是否在白名单中
     */
    bool is_ip_allowed(const std::string& ip);

    // ==================== 客户端模式（向 Skynet 发送请求） ====================
    
    /**
     * @brief 连接到 Skynet 服务器
     * @param host 主机地址
     * @param port 端口
     * @return true 连接成功
     */
    bool connect(const std::string& host, uint16_t port);
    
    /**
     * @brief 断开客户端连接
     */
    void disconnect();
    
    /**
     * @brief 检查客户端是否已连接
     */
    bool is_connected() const { return client_connected_.load(); }
    
    /**
     * @brief 同步发送消息并等待响应（阻塞）
     * @param req 请求消息
     * @param res 响应消息（输出参数）
     * @param timeout_ms 超时时间（毫秒）
     * @return true 成功收到响应
     */
    bool send_and_wait(const SkynetTcpMessage& req, SkynetTcpMessage& res, int timeout_ms = 5000);
    
    /**
     * @brief 异步发送消息（不等待响应）
     * @param msg 消息
     * @return true 发送成功
     */
    bool send_async(const SkynetTcpMessage& msg);
    
    /**
     * @brief 设置推送消息回调（处理来自 Skynet 的主动推送）
     */
    void set_push_callback(PushCallback callback) { push_callback_ = std::move(callback); }

private:
    // 获取日志记录器
    std::shared_ptr<spdlog::logger> get_logger();

    // ==================== 服务器模式内部方法 ====================
    void server_thread_func();
    void handle_server_client(SocketType client_socket, const std::string& client_ip);

    // ==================== 客户端模式内部方法 ====================
    void client_receive_thread_func();

private:
    LoggerManager& logger_manager_;
    ConnectionPool* central_pool_;
    
    // ==================== 服务器模式成员 ====================
    std::atomic<bool> server_running_{false};
    std::thread server_thread_;
    SocketType server_fd_{INVALID_SOCKET_VALUE};
    int server_port_{8889};
    
    // IP 白名单
    std::set<std::string> whitelist_;
    std::mutex whitelist_mutex_;
    
    // 请求处理回调
    RequestHandler request_handler_;

    // ==================== 客户端模式成员 ====================
    std::atomic<bool> client_connected_{false};
    std::atomic<bool> client_receive_running_{false};
    std::thread client_receive_thread_;
    SocketType client_fd_{INVALID_SOCKET_VALUE};
    std::string client_host_;
    uint16_t client_port_{0};
    
    // 发送互斥锁
    std::mutex client_send_mutex_;
    
    // 同步请求-响应机制
    std::mutex response_mutex_;
    std::condition_variable response_cv_;
    SkynetTcpMessage last_response_;
    bool has_response_{false};
    
    // 推送消息回调
    PushCallback push_callback_;
};

#endif // SKYNET_TCP_MANAGER_H
