#include "gateway_server.h"
#include "server_db.grpc.pb.h"

// ==================== 内部 TCP 服务器实现 ====================
// 此服务器专门给 Skynet 服务器使用，支持 DB 转发
// 安全措施：只接受来自已注册 Skynet 服务器 IP 的连接

void GatewayServerImpl::start_internal_tcp_server(int port)
{
    if (internal_tcp_running_.load())
    {
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn("Internal TCP server already running");
        return;
    }
    
    internal_tcp_port_ = port;
    internal_tcp_running_.store(true);
    
    internal_tcp_thread_ = std::thread(&GatewayServerImpl::internal_tcp_server_thread, this);
    
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info(
        "Internal TCP server starting on port {}", port);
}

void GatewayServerImpl::stop_internal_tcp_server()
{
    if (!internal_tcp_running_.load())
    {
        return;
    }
    
    internal_tcp_running_.store(false);
    
    if (internal_tcp_fd_ != -1)
    {
        #ifdef _WIN32
        closesocket(internal_tcp_fd_);
        #else
        close(internal_tcp_fd_);
        #endif
        internal_tcp_fd_ = -1;
    }
    
    if (internal_tcp_thread_.joinable())
    {
        internal_tcp_thread_.join();
    }
    
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Internal TCP server stopped");
}

void GatewayServerImpl::internal_tcp_server_thread()
{
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error("WSAStartup failed");
        return;
    }
    #endif
    
    // 创建 socket
    internal_tcp_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (internal_tcp_fd_ < 0)
    {
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error("Failed to create internal TCP socket");
        #ifdef _WIN32
        WSACleanup();
        #endif
        return;
    }
    
    // 设置端口重用
    int opt = 1;
    #ifdef _WIN32
    setsockopt(internal_tcp_fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    #else
    setsockopt(internal_tcp_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif
    
    // 绑定地址
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // 监听所有接口（生产环境可改为 127.0.0.1）
    server_addr.sin_port = htons(internal_tcp_port_);
    
    if (bind(internal_tcp_fd_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error(
            "Failed to bind internal TCP on port {}", internal_tcp_port_);
        #ifdef _WIN32
        closesocket(internal_tcp_fd_);
        WSACleanup();
        #else
        close(internal_tcp_fd_);
        #endif
        return;
    }
    
    if (listen(internal_tcp_fd_, 10) < 0)
    {
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error("Failed to listen on internal TCP");
        #ifdef _WIN32
        closesocket(internal_tcp_fd_);
        WSACleanup();
        #else
        close(internal_tcp_fd_);
        #endif
        return;
    }
    
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info(
        "Internal TCP server listening on port {}", internal_tcp_port_);
    
    // 接受连接
    while (internal_tcp_running_.load())
    {
        sockaddr_in client_addr{};
        #ifdef _WIN32
        int client_len = sizeof(client_addr);
        #else
        socklen_t client_len = sizeof(client_addr);
        #endif
        
        int client_socket = accept(internal_tcp_fd_, (sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0)
        {
            if (internal_tcp_running_.load())
            {
                get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn("Failed to accept internal connection");
            }
            continue;
        }
        
        // 获取客户端 IP
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::string ip_str(client_ip);
        
        // ==================== 安全校验：检查 IP 白名单 ====================
        if (!is_skynet_ip(ip_str))
        {
            get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn(
                "Rejected internal connection from unauthorized IP: {}", ip_str);
            #ifdef _WIN32
            closesocket(client_socket);
            #else
            close(client_socket);
            #endif
            continue;
        }
        
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info(
            "Accepted internal connection from Skynet: {}", ip_str);
        
        // 在线程池中处理请求
        submit_task([this, client_socket, ip_str]() {
            handle_internal_client(client_socket, ip_str);
        });
    }
    
    #ifdef _WIN32
    closesocket(internal_tcp_fd_);
    WSACleanup();
    #else
    close(internal_tcp_fd_);
    #endif
    
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Internal TCP server thread exiting");
}

void GatewayServerImpl::handle_internal_client(int client_socket, const std::string& client_ip)
{
    try
    {
        // 协议：[4字节长度][2字节类型][Protobuf数据]
        while (internal_tcp_running_.load())
        {
            // 1. 读取长度 (4 bytes, big-endian)
            uint32_t length = 0;
            int bytes_read = recv(client_socket, (char*)&length, 4, MSG_WAITALL);
            if (bytes_read != 4)
            {
                break;  // 连接关闭
            }
            length = ntohl(length);
            
            if (length < 2 || length > 1024 * 1024)
            {
                get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn(
                    "Invalid message length from {}: {}", client_ip, length);
                break;
            }
            
            // 2. 读取消息类型 (2 bytes) + 数据
            std::vector<char> buffer(length);
            int total_read = 0;
            while (total_read < (int)length)
            {
                int n = recv(client_socket, buffer.data() + total_read, length - total_read, 0);
                if (n <= 0) break;
                total_read += n;
            }
            
            if (total_read != (int)length)
            {
                break;
            }
            
            uint16_t msg_type = ((uint8_t)buffer[0] << 8) | (uint8_t)buffer[1];
            std::string payload(buffer.begin() + 2, buffer.end());
            
            get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->debug(
                "Internal request from {}: type={}, len={}", client_ip, msg_type, payload.size());
            
            // 3. 处理请求
            rpc_server::ForwardRes response;
            
            switch (msg_type)
            {
            case 20:  // REQ_DB_CREATE
                Forward_to_db_create_service(payload, &response);
                break;
            case 22:  // REQ_DB_READ
                Forward_to_db_read_service(payload, &response);
                break;
            case 24:  // REQ_DB_UPDATE
                Forward_to_db_update_service(payload, &response);
                break;
            case 26:  // REQ_DB_DELETE
                Forward_to_db_delete_service(payload, &response);
                break;
            default:
                get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn(
                    "Unknown internal message type: {}", msg_type);
                response.set_success(false);
                break;
            }
            
            // 4. 发送响应
            std::string response_data = response.response();  // ForwardRes.response 字段
            uint16_t response_type = msg_type + 1;  // 响应类型 = 请求类型 + 1
            
            uint32_t response_length = htonl(2 + response_data.size());
            send(client_socket, (const char*)&response_length, 4, 0);
            
            uint8_t type_bytes[2] = {(uint8_t)(response_type >> 8), (uint8_t)(response_type & 0xFF)};
            send(client_socket, (const char*)type_bytes, 2, 0);
            send(client_socket, response_data.data(), response_data.size(), 0);
        }
    }
    catch (const std::exception& e)
    {
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error(
            "Internal client error from {}: {}", client_ip, e.what());
    }
    
    #ifdef _WIN32
    closesocket(client_socket);
    #else
    close(client_socket);
    #endif
}

// ==================== Skynet IP 白名单管理 ====================

void GatewayServerImpl::update_skynet_whitelist()
{
    try
    {
        // 从 Central 获取 Skynet 服务器列表
        auto channel = central_connection_pool_->get_connection(rpc_server::ServerType::CENTRAL);
        auto stub = rpc_server::CentralServer::NewStub(channel);
        
        rpc_server::MultipleConnectPoorReq req;
        req.add_server_types(rpc_server::ServerType::LOGIC);  // Skynet 注册为 LOGIC 类型
        
        rpc_server::MultipleConnectPoorRes res;
        grpc::ClientContext context;
        
        grpc::Status status = stub->Get_connec_poor(&context, req, &res);
        central_connection_pool_->release_connection(rpc_server::ServerType::CENTRAL, channel);
        
        if (status.ok() && res.success())
        {
            std::lock_guard<std::mutex> lock(whitelist_mutex_);
            skynet_ip_whitelist_.clear();
            
            // 始终允许本地连接
            skynet_ip_whitelist_.insert("127.0.0.1");
            skynet_ip_whitelist_.insert("::1");
            
            for (const auto& pool : res.connect_pools())
            {
                if (pool.server_type() == rpc_server::ServerType::LOGIC)
                {
                    for (const auto& conn : pool.connect_info())
                    {
                        skynet_ip_whitelist_.insert(conn.address());
                        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info(
                            "Added Skynet IP to whitelist: {}", conn.address());
                    }
                }
            }
            
            get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info(
                "Skynet whitelist updated: {} IPs", skynet_ip_whitelist_.size());
        }
    }
    catch (const std::exception& e)
    {
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error(
            "Failed to update Skynet whitelist: {}", e.what());
    }
}

bool GatewayServerImpl::is_skynet_ip(const std::string& ip)
{
    std::lock_guard<std::mutex> lock(whitelist_mutex_);
    
    // 如果白名单为空，允许本地连接（开发模式）
    if (skynet_ip_whitelist_.empty())
    {
        return ip == "127.0.0.1" || ip == "::1";
    }
    
    return skynet_ip_whitelist_.find(ip) != skynet_ip_whitelist_.end();
}
