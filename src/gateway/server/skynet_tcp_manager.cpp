#include "skynet_tcp_manager.h"

#include <cstring>
#include <iostream>

// ==================== 构造与析构 ====================

SkynetTcpManager::SkynetTcpManager(LoggerManager& logger_manager, ConnectionPool* central_pool)
    : logger_manager_(logger_manager)
    , central_pool_(central_pool)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    
    // 默认允许本地连接
    whitelist_.insert("127.0.0.1");
    whitelist_.insert("::1");
}

SkynetTcpManager::~SkynetTcpManager()
{
    stop_server();
    disconnect();
    
#ifdef _WIN32
    WSACleanup();
#endif
}

std::shared_ptr<spdlog::logger> SkynetTcpManager::get_logger()
{
    return logger_manager_.getLogger(poor::LogCategory::APPLICATION_ACTIVITY);
}

// ==================== 公共协议方法（静态） ====================

bool SkynetTcpManager::send_message(SocketType socket, const SkynetTcpMessage& msg)
{
    // 协议：[4字节长度][2字节类型][数据]
    // 长度 = 类型(2) + 数据长度
    uint32_t content_length = static_cast<uint32_t>(2 + msg.payload.size());
    
    // 构造消息头：[length(4)][type(2)]
    uint8_t header[6];
    header[0] = static_cast<uint8_t>((content_length >> 24) & 0xFF);
    header[1] = static_cast<uint8_t>((content_length >> 16) & 0xFF);
    header[2] = static_cast<uint8_t>((content_length >> 8) & 0xFF);
    header[3] = static_cast<uint8_t>(content_length & 0xFF);
    header[4] = static_cast<uint8_t>((msg.msg_type >> 8) & 0xFF);
    header[5] = static_cast<uint8_t>(msg.msg_type & 0xFF);
    
    // 发送消息头
    if (send(socket, reinterpret_cast<const char*>(header), sizeof(header), 0) != sizeof(header))
    {
        return false;
    }
    
    // 发送数据
    if (!msg.payload.empty())
    {
        size_t total_sent = 0;
        while (total_sent < msg.payload.size())
        {
            int sent = send(socket, msg.payload.data() + total_sent, 
                           static_cast<int>(msg.payload.size() - total_sent), 0);
            if (sent <= 0)
            {
                return false;
            }
            total_sent += sent;
        }
    }
    
    return true;
}

bool SkynetTcpManager::receive_message(SocketType socket, SkynetTcpMessage& msg)
{
    // 1. 读取长度 (4 bytes, 大端序)
    uint8_t length_buf[4];
    int bytes_read = recv(socket, reinterpret_cast<char*>(length_buf), 4, MSG_WAITALL);
    if (bytes_read != 4)
    {
        return false;
    }
    
    uint32_t content_length = (static_cast<uint32_t>(length_buf[0]) << 24) |
                              (static_cast<uint32_t>(length_buf[1]) << 16) |
                              (static_cast<uint32_t>(length_buf[2]) << 8) |
                              static_cast<uint32_t>(length_buf[3]);
    
    // 安全检查：消息长度至少 2 字节（类型），最大 1MB
    if (content_length < 2 || content_length > 1024 * 1024)
    {
        return false;
    }
    
    // 2. 读取类型 (2 bytes)
    uint8_t type_buf[2];
    if (recv(socket, reinterpret_cast<char*>(type_buf), 2, MSG_WAITALL) != 2)
    {
        return false;
    }
    
    msg.msg_type = (static_cast<uint16_t>(type_buf[0]) << 8) |
                   static_cast<uint16_t>(type_buf[1]);
    
    // 3. 读取数据
    uint32_t data_length = content_length - 2;
    if (data_length > 0)
    {
        msg.payload.resize(data_length);
        size_t total_read = 0;
        while (total_read < data_length)
        {
            int n = recv(socket, &msg.payload[0] + total_read, 
                        static_cast<int>(data_length - total_read), 0);
            if (n <= 0)
            {
                return false;
            }
            total_read += n;
        }
    }
    else
    {
        msg.payload.clear();
    }
    
    return true;
}

// ==================== 服务器模式 ====================

bool SkynetTcpManager::start_server(int port)
{
    if (server_running_.load())
    {
        get_logger()->warn("SkynetTcpManager server already running");
        return false;
    }
    
    server_port_ = port;
    server_running_.store(true);
    
    server_thread_ = std::thread(&SkynetTcpManager::server_thread_func, this);
    
    get_logger()->info("SkynetTcpManager server starting on port {}", port);
    return true;
}

void SkynetTcpManager::stop_server()
{
    if (!server_running_.load())
    {
        return;
    }
    
    server_running_.store(false);
    
    // 关闭服务器 socket 以中断 accept
    if (server_fd_ != INVALID_SOCKET_VALUE)
    {
        closesocket(server_fd_);
        server_fd_ = INVALID_SOCKET_VALUE;
    }
    
    if (server_thread_.joinable())
    {
        server_thread_.join();
    }
    
    get_logger()->info("SkynetTcpManager server stopped");
}

void SkynetTcpManager::update_whitelist()
{
    if (!central_pool_)
    {
        get_logger()->warn("No central pool configured, cannot update whitelist");
        return;
    }
    
    try
    {
        auto channel = central_pool_->get_connection(rpc_server::ServerType::CENTRAL);
        auto stub = rpc_server::CentralServer::NewStub(channel);
        
        rpc_server::MultipleConnectPoorReq req;
        req.add_server_types(rpc_server::ServerType::LOGIC);  // Skynet 注册为 LOGIC 类型
        
        rpc_server::MultipleConnectPoorRes res;
        grpc::ClientContext context;
        
        grpc::Status status = stub->Get_connec_poor(&context, req, &res);
        central_pool_->release_connection(rpc_server::ServerType::CENTRAL, channel);
        
        if (status.ok() && res.success())
        {
            std::lock_guard<std::mutex> lock(whitelist_mutex_);
            whitelist_.clear();
            
            // 始终允许本地连接
            whitelist_.insert("127.0.0.1");
            whitelist_.insert("::1");
            
            for (const auto& pool : res.connect_pools())
            {
                if (pool.server_type() == rpc_server::ServerType::LOGIC)
                {
                    for (const auto& conn : pool.connect_info())
                    {
                        whitelist_.insert(conn.address());
                        get_logger()->info("Added Skynet IP to whitelist: {}", conn.address());
                    }
                }
            }
            
            get_logger()->info("Skynet whitelist updated: {} IPs", whitelist_.size());
        }
    }
    catch (const std::exception& e)
    {
        get_logger()->error("Failed to update Skynet whitelist: {}", e.what());
    }
}

void SkynetTcpManager::add_to_whitelist(const std::string& ip)
{
    std::lock_guard<std::mutex> lock(whitelist_mutex_);
    whitelist_.insert(ip);
}

bool SkynetTcpManager::is_ip_allowed(const std::string& ip)
{
    std::lock_guard<std::mutex> lock(whitelist_mutex_);
    
    // 如果白名单只有默认的本地地址，允许所有本地连接（开发模式）
    if (whitelist_.size() <= 2)
    {
        return ip == "127.0.0.1" || ip == "::1";
    }
    
    return whitelist_.find(ip) != whitelist_.end();
}

void SkynetTcpManager::server_thread_func()
{
    // 创建 socket
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == INVALID_SOCKET_VALUE)
    {
        get_logger()->error("Failed to create server socket");
        return;
    }
    
    // 设置端口重用
    int opt = 1;
#ifdef _WIN32
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    // 绑定地址
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(static_cast<uint16_t>(server_port_));
    
    if (bind(server_fd_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
    {
        get_logger()->error("Failed to bind server on port {}", server_port_);
        closesocket(server_fd_);
        server_fd_ = INVALID_SOCKET_VALUE;
        return;
    }
    
    if (listen(server_fd_, 10) < 0)
    {
        get_logger()->error("Failed to listen on port {}", server_port_);
        closesocket(server_fd_);
        server_fd_ = INVALID_SOCKET_VALUE;
        return;
    }
    
    get_logger()->info("SkynetTcpManager server listening on port {}", server_port_);
    
    // 接受连接
    while (server_running_.load())
    {
        sockaddr_in client_addr{};
#ifdef _WIN32
        int client_len = sizeof(client_addr);
#else
        socklen_t client_len = sizeof(client_addr);
#endif
        
        SocketType client_socket = accept(server_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        
        if (client_socket == INVALID_SOCKET_VALUE)
        {
            if (server_running_.load())
            {
                get_logger()->warn("Failed to accept connection");
            }
            continue;
        }
        
        // 获取客户端 IP
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::string ip_str(client_ip);
        
        // IP 白名单校验
        if (!is_ip_allowed(ip_str))
        {
            get_logger()->warn("Rejected connection from unauthorized IP: {}", ip_str);
            closesocket(client_socket);
            continue;
        }
        
        get_logger()->info("Accepted connection from Skynet: {}", ip_str);
        
        // 在新线程中处理客户端
        std::thread(&SkynetTcpManager::handle_server_client, this, client_socket, ip_str).detach();
    }
    
    if (server_fd_ != INVALID_SOCKET_VALUE)
    {
        closesocket(server_fd_);
        server_fd_ = INVALID_SOCKET_VALUE;
    }
    
    get_logger()->info("SkynetTcpManager server thread exiting");
}

void SkynetTcpManager::handle_server_client(SocketType client_socket, const std::string& client_ip)
{
    try
    {
        while (server_running_.load())
        {
            // 接收请求
            SkynetTcpMessage request;
            if (!receive_message(client_socket, request))
            {
                break;  // 连接关闭或错误
            }
            
            get_logger()->debug("Server received from {}: type={}, len={}", 
                               client_ip, request.msg_type, request.payload.size());
            
            // 调用处理回调
            std::string response_data;
            if (request_handler_)
            {
                response_data = request_handler_(request.msg_type, request.payload);
            }
            else
            {
                get_logger()->warn("No request handler set");
            }
            
            // 发送响应（响应类型 = 请求类型 + 1）
            SkynetTcpMessage response(request.msg_type + 1, std::move(response_data));
            if (!send_message(client_socket, response))
            {
                get_logger()->warn("Failed to send response to {}", client_ip);
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        get_logger()->error("Server client error from {}: {}", client_ip, e.what());
    }
    
    closesocket(client_socket);
}

// ==================== 客户端模式 ====================

bool SkynetTcpManager::connect(const std::string& host, uint16_t port)
{
    if (client_connected_.load())
    {
        get_logger()->warn("Client already connected");
        return true;
    }
    
    client_host_ = host;
    client_port_ = port;
    
    // 创建 socket
    client_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd_ == INVALID_SOCKET_VALUE)
    {
        get_logger()->error("Failed to create client socket");
        return false;
    }
    
    // 设置服务器地址
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0)
    {
        get_logger()->error("Invalid address: {}", host);
        closesocket(client_fd_);
        client_fd_ = INVALID_SOCKET_VALUE;
        return false;
    }
    
    // 连接
    if (::connect(client_fd_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
    {
        get_logger()->error("Failed to connect to {}:{}", host, port);
        closesocket(client_fd_);
        client_fd_ = INVALID_SOCKET_VALUE;
        return false;
    }
    
    client_connected_.store(true);
    get_logger()->info("Connected to Skynet: {}:{}", host, port);
    
    // 启动接收线程
    client_receive_running_.store(true);
    client_receive_thread_ = std::thread(&SkynetTcpManager::client_receive_thread_func, this);
    
    return true;
}

void SkynetTcpManager::disconnect()
{
    if (!client_connected_.load())
    {
        return;
    }
    
    // 停止接收线程
    client_receive_running_.store(false);
    
    // 关闭 socket
    if (client_fd_ != INVALID_SOCKET_VALUE)
    {
        closesocket(client_fd_);
        client_fd_ = INVALID_SOCKET_VALUE;
    }
    
    if (client_receive_thread_.joinable())
    {
        client_receive_thread_.join();
    }
    
    client_connected_.store(false);
    get_logger()->info("Disconnected from Skynet");
}

bool SkynetTcpManager::send_and_wait(const SkynetTcpMessage& req, SkynetTcpMessage& res, int timeout_ms)
{
    if (!client_connected_.load())
    {
        get_logger()->error("Client not connected");
        return false;
    }
    
    // 发送请求
    {
        std::lock_guard<std::mutex> lock(client_send_mutex_);
        if (!send_message(client_fd_, req))
        {
            get_logger()->error("Failed to send request");
            return false;
        }
    }
    
    // 等待响应
    std::unique_lock<std::mutex> lock(response_mutex_);
    has_response_ = false;
    
    if (response_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), 
        [this] { return has_response_; }))
    {
        res = last_response_;
        return true;
    }
    
    get_logger()->warn("Timeout waiting for response");
    return false;
}

bool SkynetTcpManager::send_async(const SkynetTcpMessage& msg)
{
    if (!client_connected_.load())
    {
        get_logger()->error("Client not connected");
        return false;
    }
    
    std::lock_guard<std::mutex> lock(client_send_mutex_);
    return send_message(client_fd_, msg);
}

void SkynetTcpManager::client_receive_thread_func()
{
    get_logger()->debug("Client receive thread started");
    
    while (client_receive_running_.load() && client_connected_.load())
    {
        SkynetTcpMessage msg;
        if (!receive_message(client_fd_, msg))
        {
            if (client_receive_running_.load())
            {
                get_logger()->warn("Client receive failed, disconnecting...");
                client_connected_.store(false);
            }
            break;
        }
        
        get_logger()->debug("Client received: type={}, len={}", msg.msg_type, msg.payload.size());
        
        // 判断是响应还是推送
        // 响应类型通常是请求类型 + 1（奇数），推送类型 >= 200
        if (msg.msg_type >= 200)
        {
            // 推送消息
            if (push_callback_)
            {
                push_callback_(msg);
            }
        }
        else
        {
            // 响应消息
            std::lock_guard<std::mutex> lock(response_mutex_);
            last_response_ = std::move(msg);
            has_response_ = true;
            response_cv_.notify_one();
        }
    }
    
    get_logger()->debug("Client receive thread stopped");
}
