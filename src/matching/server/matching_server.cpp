#include "matching_server.h"

// 构造函数 - 继承 BaseServer
MatchingServerImpl::MatchingServerImpl(LoggerManager& logger_manager_, const std::string address, std::string port)
    : BaseServer(rpc_server::ServerType::MATCHING, address, port, logger_manager_, 4),  // 4 个线程
      battle_connection_pool(10),
      login_connection_pool(10)
{
    // Redis 连接将在 on_start() 中初始化
}

// 析构函数
MatchingServerImpl::~MatchingServerImpl()
{
    // 停止连接池更新线程
    pool_update_running_.store(false);
    if (update_pool_thread_.joinable())
    {
        update_pool_thread_.join();
    }
    
    log_shutdown("MatchingServer stopped");
}

// ==================== BaseServer 钩子方法 ====================

bool MatchingServerImpl::on_start()
{
    log_startup("MatchingServer initializing...");
    
    // 连接 Redis
    redis_client.get_client()->connect("127.0.0.1", 6379);
    log_startup("Redis connection successful");
    
    log_startup("MatchingServer initialized successfully");
    return true;
}

void MatchingServerImpl::on_stop()
{
    log_shutdown("MatchingServer shutting down...");
    
    // 停止连接池更新线程
    pool_update_running_.store(false);
    if (update_pool_thread_.joinable())
    {
        update_pool_thread_.join();
    }
    
    // 清空匹配队列
    {
        std::lock_guard<std::mutex> lock(match_mutex);
        match_queues.clear();
    }
    
    log_shutdown("MatchingServer shutdown complete");
}

void MatchingServerImpl::on_registered(bool success)
{
    if (success)
    {
        // 注册成功后，初始化连接池
        Init_connection_pool();
        
        // 启动定时更新连接池的线程
        pool_update_running_.store(true);
        update_pool_thread_ = std::thread(&MatchingServerImpl::Update_connection_pool, this);
        
        log_startup("Matching connection pools initialized and update thread started");
    }
    else
    {
        log_startup("Failed to register, skipping connection pool initialization");
    }
}

// ==================== 连接池管理 ====================

void MatchingServerImpl::Init_connection_pool()
{
    try
    {
        auto channel = central_connection_pool_->get_connection(rpc_server::ServerType::CENTRAL);
        auto stub = rpc_server::CentralServer::NewStub(channel);
        
        rpc_server::MultipleConnectPoorReq req;
        req.add_server_types(rpc_server::ServerType::LOGIN);
        req.add_server_types(rpc_server::ServerType::BATTLE);
        
        rpc_server::MultipleConnectPoorRes res;
        grpc::ClientContext context;
        
        grpc::Status status = stub->Get_connec_poor(&context, req, &res);
        
        central_connection_pool_->release_connection(rpc_server::ServerType::CENTRAL, channel);
        
        if (status.ok() && res.success())
        {
            for (const rpc_server::ConnectPool& connect_pool : res.connect_pools())
            {
                for (const rpc_server::ConnectInfo& conn_info : connect_pool.connect_info())
                {
                    switch (connect_pool.server_type())
                    {
                    case rpc_server::ServerType::LOGIN:
                        login_connection_pool.add_server(
                            rpc_server::ServerType::LOGIN,
                            conn_info.address(),
                            std::to_string(conn_info.port())
                        );
                        break;
                        
                    case rpc_server::ServerType::BATTLE:
                        battle_connection_pool.add_server(
                            rpc_server::ServerType::BATTLE,
                            conn_info.address(),
                            std::to_string(conn_info.port())
                        );
                        break;
                        
                    default:
                        break;
                    }
                }
            }
            
            get_logger(poor::LogCategory::CONNECTION_POOL)->info("Matching server updated connection pools successfully");
        }
        else
        {
            get_logger(poor::LogCategory::CONNECTION_POOL)->error("Failed to get connection pools information");
        }
    }
    catch (const std::exception& e)
    {
        get_logger(poor::LogCategory::CONNECTION_POOL)->error("Exception during connection pool initialization: {}", e.what());
    }
}

void MatchingServerImpl::Update_connection_pool()
{
    while (pool_update_running_.load())
    {
        std::this_thread::sleep_for(std::chrono::minutes(5));
        
        if (pool_update_running_.load())
        {
            Init_connection_pool();
        }
    }
}

// ==================== gRPC 服务接口 ====================

grpc::Status MatchingServerImpl::MatchPlayer(
    grpc::ServerContext* context [[maybe_unused]],
    const rpc_server::MatchPlayerReq* req,
    grpc::ServerWriter<rpc_server::MatchPlayerRes>* writer)
{
    // 1. 获取玩家信息（从请求或通过逻辑服务器）
    PlayerInfo info;
    info.player_id = req->player_id();  // int32_t 类型
    // TODO: 通过 RPC 获取玩家段位和分数
    // 示例：假设从请求中直接获取
    // info.rank = req->rank();
    // info.score = req->score();
    info.rank = 1;  // 临时值
    info.score = 1000;  // 临时值

    // 2. 加入匹配队列
    {
        std::lock_guard<std::mutex> lock(match_mutex);
        match_queues[info.rank].push_back(info);
        
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info(
            "Player {} joined matching queue (rank: {})", 
            info.player_id, 
            info.rank
        );
    }

    // 3. 匹配逻辑（简化版本）
    // TODO: 实现真实的匹配算法
    // - 等待匹配
    // - 找到对手
    // - 创建战斗房间
    // - 通知双方玩家
    
    rpc_server::MatchPlayerRes res;
    res.set_success(true);
    res.set_message("Matching...");
    res.set_status(0);  // 0: 加入队列成功
    
    writer->Write(res);

    return grpc::Status::OK;
}

grpc::Status MatchingServerImpl::CancelMatch(
    grpc::ServerContext* context [[maybe_unused]],
    const rpc_server::CancelMatchReq* req,
    rpc_server::CancelMatchRes* res)
{
    int32_t player_id = req->player_id();  // int32_t 类型
    
    std::lock_guard<std::mutex> lock(match_mutex);
    
    // 遍历所有段位的匹配队列
    for (auto& [rank, queue] : match_queues)
    {
        auto it = std::find_if(queue.begin(), queue.end(), [&](const PlayerInfo& p) {
            return p.player_id == player_id;
        });

        if (it != queue.end())
        {
            queue.erase(it);
            res->set_success(true);
            res->set_message("已取消匹配");
            
            get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info(
                "Player {} cancelled matching (rank: {})", 
                player_id, 
                rank
            );
            
            return grpc::Status::OK;
        }
    }
    
    res->set_success(false);
    res->set_message("未找到匹配中的玩家");
    
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->warn(
        "Failed to cancel match for player {}: not found in queue", 
        player_id
    );
    
    return grpc::Status::OK;
}
