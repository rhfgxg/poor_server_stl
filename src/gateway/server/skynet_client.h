#ifndef SKYNET_CLIENT_H
#define SKYNET_CLIENT_H

#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

// Skynet 消息结构
struct SkynetMessage {
    uint32_t length;        // 消息长度
    uint16_t msg_type;      // 消息类型
    std::string data;       // 消息数据
    
    SkynetMessage() : length(0), msg_type(0) {}
    SkynetMessage(uint16_t type, const std::string& d) 
        : length(static_cast<uint32_t>(d.size())), msg_type(type), data(d) {}
};

// Skynet 消息类型定义
enum class SkynetMsgType : uint16_t {
    ENTER_GAME = 1,
    LEAVE_GAME = 2,
    PLAYER_ACTION = 3,
    GET_PLAYER_STATUS = 4,
    
    // 响应类型（请求类型 + 100）
    ENTER_GAME_RES = 101,
    LEAVE_GAME_RES = 102,
    PLAYER_ACTION_RES = 103,
    GET_PLAYER_STATUS_RES = 104,
    
    // 推送消息
    PUSH_GAME_STATE = 200,
    PUSH_NOTIFICATION = 201,
};

// SkynetClient - Gateway 用于连接 Skynet 的客户端
class SkynetClient {
public:
    SkynetClient(const std::string& host, uint16_t port);
    ~SkynetClient();

    // 连接到 Skynet
    bool connect();
    
    // 断开连接
    void disconnect();
    
    // 检查是否已连接
    bool is_connected() const;
    
    // 同步发送消息并等待响应（阻塞）
    bool send_and_wait(const SkynetMessage& req, SkynetMessage& res, int timeout_ms = 5000);
    
    // 异步发送消息（不等待响应）
    bool send_async(const SkynetMessage& msg);
    
    // 设置推送消息回调（处理来自 Skynet 的主动推送）
    using PushCallback = std::function<void(const SkynetMessage&)>;
    void set_push_callback(PushCallback callback);

private:
    // 接收线程函数
    void receive_thread_func();
    
    // 接收完整消息
    bool receive_message(SkynetMessage& msg);
    
    // 发送消息
    bool send_message(const SkynetMessage& msg);

    // ==================== 成员变量 ====================
    
    std::string host_;
    uint16_t port_;
    
    int sockfd_;                                // Socket 文件描述符
    std::atomic<bool> connected_;               // 连接状态
    std::atomic<bool> receive_running_;         // 接收线程运行状态
    
    std::thread receive_thread_;                // 接收线程
    PushCallback push_callback_;                // 推送消息回调
    
    std::mutex send_mutex_;                     // 发送互斥锁
    std::mutex receive_mutex_;                  // 接收互斥锁
    
    // 同步请求-响应机制
    std::mutex response_mutex_;
    std::condition_variable response_cv_;
    SkynetMessage last_response_;
    bool has_response_;
};

#endif // SKYNET_CLIENT_H
