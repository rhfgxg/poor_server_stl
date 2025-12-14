-- Redis 配置文件

return {
    -- 本地 Redis（单机，会话/临时数据）
    local = {
        host = "127.0.0.1",
        port = 6379,
        db = 0,
        password = nil,
        timeout_ms = 2000,
        
        -- 连接池配置
        pool_size = 10,
        max_idle_time = 300
    },
    
    -- 集群 Redis（共享/持久数据）
    cluster = {
        -- 集群节点列表
        nodes = {
            "127.0.0.1:7000",
            "127.0.0.1:7001",
            "127.0.0.1:7002",
            "127.0.0.1:7003",
            "127.0.0.1:7004",
            "127.0.0.1:7005"
        },
        
        -- 单机模式（开发环境）
        -- 如果没有集群，可以只配置一个节点
        -- nodes = { "127.0.0.1:6379" },
        
        password = nil,
        timeout_ms = 2000,
        
        -- 连接池配置
        pool_size = 20,
        max_idle_time = 600
    },
    
    -- C++ Redis Server 配置（给 Skynet 连接）
    server = {
        host = "127.0.0.1",
        port = 8003,
        max_connections = 100
    },
    
    -- 数据过期策略
    ttl = {
        -- 会话数据（本地 Redis）
        session = 3600,
        temp = 1800,
        
        -- 玩家数据（集群 Redis）
        player_data = 86400,
        battle_data = 7200,
        cache_data = 3600
    },
    
    -- 性能配置
    performance = {
        -- Pipeline 批量大小
        pipeline_batch_size = 100,
        
        -- 重试配置
        max_retries = 3,
        retry_interval_ms = 100,
        
        -- 慢查询阈值（毫秒）
        slow_log_threshold = 100
    }
}
