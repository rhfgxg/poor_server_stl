#include "skynet_client.h"
#include <cstring>
#include <iostream>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

SkynetClient::SkynetClient(const std::string& host, uint16_t port)
    : host_(host)
    , port_(port)
    , sockfd_(INVALID_SOCKET)
    , connected_(false)
    , receive_running_(false)
    , has_response_(false)
{
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
}

SkynetClient::~SkynetClient()
{
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool SkynetClient::connect()
{
    if (connected_.load()) {
        return true;
    }

    // 创建 socket
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ == INVALID_SOCKET) {
        std::cerr << "[SkynetClient] Failed to create socket" << std::endl;
        return false;
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    
    if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "[SkynetClient] Invalid address: " << host_ << std::endl;
        closesocket(sockfd_);
        sockfd_ = INVALID_SOCKET;
        return false;
    }

    // 连接
    if (::connect(sockfd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "[SkynetClient] Failed to connect to " << host_ << ":" << port_ << std::endl;
        closesocket(sockfd_);
        sockfd_ = INVALID_SOCKET;
        return false;
    }

    connected_.store(true);
    std::cout << "[SkynetClient] Connected to Skynet: " << host_ << ":" << port_ << std::endl;

    // 启动接收线程
    receive_running_.store(true);
    receive_thread_ = std::thread(&SkynetClient::receive_thread_func, this);

    return true;
}

void SkynetClient::disconnect()
{
    if (!connected_.load()) {
        return;
    }

    // 停止接收线程
    receive_running_.store(false);
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }

    // 关闭 socket
    if (sockfd_ != INVALID_SOCKET) {
        closesocket(sockfd_);
        sockfd_ = INVALID_SOCKET;
    }

    connected_.store(false);
    std::cout << "[SkynetClient] Disconnected from Skynet" << std::endl;
}

bool SkynetClient::is_connected() const
{
    return connected_.load();
}

bool SkynetClient::send_and_wait(const SkynetMessage& req, SkynetMessage& res, int timeout_ms)
{
    if (!send_message(req)) {
        return false;
    }

    // 等待响应
    std::unique_lock<std::mutex> lock(response_mutex_);
    has_response_ = false;

    if (response_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), 
        [this] { return has_response_; })) {
        res = last_response_;
        return true;
    }

    std::cerr << "[SkynetClient] Timeout waiting for response" << std::endl;
    return false;
}

bool SkynetClient::send_async(const SkynetMessage& msg)
{
    return send_message(msg);
}

void SkynetClient::set_push_callback(PushCallback callback)
{
    push_callback_ = callback;
}

bool SkynetClient::send_message(const SkynetMessage& msg)
{
    if (!connected_.load()) {
        std::cerr << "[SkynetClient] Not connected" << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(send_mutex_);

    // 构造消息：[length(4)][type(2)][data]
    uint32_t total_length = sizeof(uint16_t) + static_cast<uint32_t>(msg.data.size());
    
    uint8_t header[6];
    header[0] = (total_length >> 24) & 0xFF;
    header[1] = (total_length >> 16) & 0xFF;
    header[2] = (total_length >> 8) & 0xFF;
    header[3] = total_length & 0xFF;
    header[4] = (msg.msg_type >> 8) & 0xFF;
    header[5] = msg.msg_type & 0xFF;

    // 发送消息头
    if (send(sockfd_, reinterpret_cast<const char*>(header), sizeof(header), 0) != sizeof(header)) {
        std::cerr << "[SkynetClient] Failed to send header" << std::endl;
        return false;
    }

    // 发送消息数据
    if (!msg.data.empty()) {
        if (send(sockfd_, msg.data.c_str(), msg.data.size(), 0) != static_cast<int>(msg.data.size())) {
            std::cerr << "[SkynetClient] Failed to send data" << std::endl;
            return false;
        }
    }

    return true;
}

bool SkynetClient::receive_message(SkynetMessage& msg)
{
    // 读取长度（4 字节）
    uint8_t length_buf[4];
    if (recv(sockfd_, reinterpret_cast<char*>(length_buf), 4, MSG_WAITALL) != 4) {
        return false;
    }

    uint32_t length = (static_cast<uint32_t>(length_buf[0]) << 24) |
                      (static_cast<uint32_t>(length_buf[1]) << 16) |
                      (static_cast<uint32_t>(length_buf[2]) << 8) |
                      static_cast<uint32_t>(length_buf[3]);

    // 读取消息类型（2 字节）
    uint8_t type_buf[2];
    if (recv(sockfd_, reinterpret_cast<char*>(type_buf), 2, MSG_WAITALL) != 2) {
        return false;
    }

    uint16_t msg_type = (static_cast<uint16_t>(type_buf[0]) << 8) |
                        static_cast<uint16_t>(type_buf[1]);

    // 读取数据
    uint32_t data_length = length - 2;  // length 包含了 type 的 2 字节
    std::string data;
    if (data_length > 0) {
        data.resize(data_length);
        if (recv(sockfd_, &data[0], data_length, MSG_WAITALL) != static_cast<int>(data_length)) {
            return false;
        }
    }

    msg.length = length;
    msg.msg_type = msg_type;
    msg.data = std::move(data);

    return true;
}

void SkynetClient::receive_thread_func()
{
    std::cout << "[SkynetClient] Receive thread started" << std::endl;

    while (receive_running_.load() && connected_.load()) {
        SkynetMessage msg;
        if (!receive_message(msg)) {
            if (receive_running_.load()) {
                std::cerr << "[SkynetClient] Receive failed, disconnecting..." << std::endl;
                connected_.store(false);
            }
            break;
        }

        // 判断是响应还是推送
        if (msg.msg_type >= 100 && msg.msg_type < 200) {
            // 响应消息
            std::lock_guard<std::mutex> lock(response_mutex_);
            last_response_ = msg;
            has_response_ = true;
            response_cv_.notify_one();
        } else if (msg.msg_type >= 200) {
            // 推送消息
            if (push_callback_) {
                push_callback_(msg);
            }
        }
    }

    std::cout << "[SkynetClient] Receive thread stopped" << std::endl;
}
