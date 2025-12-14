#include "redis_server.h"
#include "config_manager.h"
#include "redis_client.h"  // 使用现有的 RedisClient
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

// ==================== 构造/析构 ====================

RedisServer::RedisServer(LoggerManager& logger_manager, const std::string& address, const std::string& port)
    : BaseServer(rpc_server::ServerType::REDIS, address, port, logger_manager, 4),
      socket_fd_(-1),
      running_(false)
{
    log_startup("RedisServer constructed");
}

RedisServer::~RedisServer()
{
    log_shutdown("RedisServer stopped");
}

// ==================== BaseServer 钩子方法 ====================

bool RedisServer::on_start()
{
    log_startup("RedisServer initializing...");
    
    // 1. 初始化 Redis 连接
    if (!init_redis_connections())
    {
        log_startup("Failed to initialize Redis connections");
        return false;
    }
    
    // 2. 启动 Socket 监听（给 Skynet）
    if (!start_socket_listener())
    {
        log_startup("Failed to start socket listener");
        return false;
    }
    
    log_startup("RedisServer initialized successfully");
    return true;
}

void RedisServer::on_stop()
{
    log_shutdown("RedisServer shutting down...");
    
    // 1. 停止 Socket 监听
    running_ = false;
    
    // 安全地关闭 socket（检查是否已初始化）
    if (socket_fd_ >= 0)
    {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    
    // 安全地停止监听线程
    if (socket_listener_thread_.joinable())
    {
        socket_listener_thread_.join();
    }
    
    // 2. 关闭所有客户端连接
    {
        std::unique_lock lock(clients_mutex_);
        for (auto& [fd, thread] : client_threads_)
        {
            close(fd);
            if (thread.joinable())
            {
                thread.join();
            }
        }
        client_threads_.clear();
    }
    
    // 3. 断开 Redis 连接（智能指针自动释放，需要检查是否已初始化）
    if (local_redis_)
    {
        local_redis_.reset();
    }
    
    if (cluster_redis_)
    {
        cluster_redis_.reset();
    }
    
    log_shutdown("RedisServer shutdown complete");
}

// ==================== Redis 连接管理 ====================

bool RedisServer::init_redis_connections()
{
    try
    {
        ConfigManager config_manager;
        std::vector<std::string> config_paths = {
            "config/cfg_redis.lua",              // 相对于可执行文件目录
            "../config/cfg_redis.lua",            // 向上一级
            "../../config/cfg_redis.lua",         // 向上两级
            "../../../config/cpp/cfg_server/cfg_redis.lua", // 项目根目录
            "config/cpp/cfg_server/cfg_redis.lua" // 从项目根运行
        };
        
        if (!config_manager.load_from_paths(config_paths))
        {
            log_startup("Failed to load Redis config");
            // 使用默认配置继续，而不是失败
            log_startup("Using default Redis configuration");
        }
        
        // 初始化本地 Redis（使用现有的 RedisClient）
        local_redis_ = std::make_shared<RedisClient>();
        
        auto local_host = config_manager.get_string("local.host").value_or("127.0.0.1");
        auto local_port = static_cast<std::size_t>(config_manager.get_int("local.port").value_or(6379));
        auto local_db = static_cast<std::uint32_t>(config_manager.get_int("local.db").value_or(0));
        
        log_startup("Connecting to local Redis: " + local_host + ":" + std::to_string(local_port) + " db=" + std::to_string(local_db));
        
        if (!local_redis_->connect(local_host, local_port, local_db))
        {
            log_startup("Failed to connect to local Redis - make sure Redis is running");
            return false;
        }
        
        log_startup("Connected to local Redis: " + local_host + ":" + std::to_string(local_port));
        
        // 初始化集群 Redis（如果配置了）
        // 简化配置读取：单机模式使用本地 Redis
        std::vector<std::string> cluster_nodes;
        cluster_nodes.push_back("127.0.0.1:6379");  // 默认使用本地 Redis
        
        cluster_redis_ = std::make_shared<RedisClusterClient>();
        if (!cluster_redis_->connect(cluster_nodes))
        {
            log_startup("Failed to connect to Redis cluster");
            // 集群连接失败不影响启动，可以降级为只使用本地 Redis
            cluster_redis_.reset();
        }
        else
        {
            log_startup("Connected to Redis cluster (single node mode)");
        }
        
        return true;
    }
    catch (const std::exception& e)
    {
        log_startup(std::string("Redis initialization failed: ") + e.what());
        return false;
    }
}

std::shared_ptr<RedisClient> RedisServer::get_local_redis()
{
    std::shared_lock lock(local_redis_mutex_);
    return local_redis_;
}

std::shared_ptr<RedisClusterClient> RedisServer::get_cluster_redis()
{
    std::shared_lock lock(cluster_redis_mutex_);
    return cluster_redis_;
}

// ==================== TCP Socket 服务（给 Skynet） ====================

bool RedisServer::start_socket_listener()
{
    // 创建 Socket
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0)
    {
        log_startup("Failed to create socket");
        return false;
    }
    
    // 设置 SO_REUSEADDR
    int opt = 1;
    setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 绑定地址
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8003);  // Redis 服务器端口
    
    if (bind(socket_fd_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        log_startup("Failed to bind socket");
        close(socket_fd_);
        return false;
    }
    
    // 监听
    if (listen(socket_fd_, 10) < 0)
    {
        log_startup("Failed to listen on socket");
        close(socket_fd_);
        return false;
    }
    
    log_startup("Socket listener started on port 8003");
    
    // 启动监听线程
    running_ = true;
    socket_listener_thread_ = std::thread([this]() {
        while (running_)
        {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(socket_fd_, (sockaddr*)&client_addr, &client_len);
            if (client_fd < 0)
            {
                if (running_)
                {
                    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error("Failed to accept client");
                }
                continue;
            }
            
            get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info(
                "New Skynet client connected: fd={}", client_fd);
            
            // 为每个客户端创建线程
            std::unique_lock lock(clients_mutex_);
            client_threads_[client_fd] = std::thread([this, client_fd]() {
                handle_socket_connection(client_fd);
            });
            client_threads_[client_fd].detach();
        }
    });
    
    return true;
}

void RedisServer::handle_socket_connection(int client_fd)
{
    char buffer[4096];
    
    while (running_)
    {
        // 读取长度头（4字节）
        ssize_t n = recv(client_fd, buffer, 4, MSG_WAITALL);
        if (n <= 0)
        {
            break;
        }
        
        uint32_t length = ntohl(*reinterpret_cast<uint32_t*>(buffer));
        
        if (length > sizeof(buffer) - 4)
        {
            get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error(
                "Message too large: {} bytes", length);
            break;
        }
        
        // 读取完整消息
        n = recv(client_fd, buffer + 4, length, MSG_WAITALL);
        if (n <= 0)
        {
            break;
        }
        
        std::string message(buffer + 4, length);
        handle_socket_message(client_fd, message);
    }
    
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Client disconnected: fd={}", client_fd);
    close(client_fd);
    
    // 从客户端列表移除
    std::unique_lock lock(clients_mutex_);
    client_threads_.erase(client_fd);
}

void RedisServer::handle_socket_message(int client_fd, const std::string& message)
{
    try
    {
        // 解析 JSON 请求
        auto request = nlohmann::json::parse(message);
        
        std::string method = request["method"];
        auto params = request["params"];
        
        nlohmann::json response;
        response["success"] = true;
        
        // 根据方法调用相应的 Redis 操作
        if (method == "set")
        {
            std::string redis_type = params["redis_type"];
            std::string key = params["key"];
            std::string value = params["value"];
            int ttl = params.value("ttl", 0);
            
            if (redis_type == "LOCAL")
            {
                auto redis = get_local_redis();
                if (ttl > 0)
                {
                    redis->set_with_expire(key, value, std::chrono::seconds(ttl));
                }
                else
                {
                    // 使用底层客户端进行 set 操作
                    auto client = redis->get_client();
                    client->set(key, value);
                    client->sync_commit();
                }
            }
            else if (redis_type == "CLUSTER")
            {
                auto redis = get_cluster_redis();
                if (redis)
                {
                    if (ttl > 0)
                    {
                        redis->set_with_expire(key, value, std::chrono::seconds(ttl));
                    }
                    else
                    {
                        redis->set(key, value);
                    }
                }
                else
                {
                    response["success"] = false;
                    response["error"] = "Cluster Redis not available";
                }
            }
            
            response["result"] = "OK";
        }
        else if (method == "get")
        {
            std::string redis_type = params["redis_type"];
            std::string key = params["key"];
            
            std::optional<std::string> value;
            
            if (redis_type == "LOCAL")
            {
                value = get_local_redis()->get_string(key);
            }
            else if (redis_type == "CLUSTER")
            {
                auto redis = get_cluster_redis();
                if (redis)
                {
                    value = redis->get(key);
                }
            }
            
            if (value)
            {
                response["result"] = *value;
            }
            else
            {
                response["result"] = nullptr;
            }
        }
        else if (method == "del")
        {
            std::string redis_type = params["redis_type"];
            std::string key = params["key"];
            
            if (redis_type == "LOCAL")
            {
                get_local_redis()->delete_key(key);
            }
            else if (redis_type == "CLUSTER")
            {
                auto redis = get_cluster_redis();
                if (redis)
                {
                    redis->del(key);
                }
            }
            
            response["result"] = "OK";
        }
        else
        {
            response["success"] = false;
            response["error"] = "Unknown method: " + method;
        }
        
        send_socket_response(client_fd, response);
    }
    catch (const std::exception& e)
    {
        nlohmann::json error_response;
        error_response["success"] = false;
        error_response["error"] = e.what();
        
        send_socket_response(client_fd, error_response);
    }
}

// ==================== 等待停止 ====================

void RedisServer::wait_for_shutdown()
{
    // 等待运行标志变为 false
    while (running_.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void RedisServer::send_socket_response(int client_fd, const nlohmann::json& response)
{
    std::string response_str = response.dump();
    
    // 打包：[length(4)][json_body]
    uint32_t length = htonl(response_str.size());
    
    send(client_fd, &length, 4, 0);
    send(client_fd, response_str.data(), response_str.size(), 0);
}
