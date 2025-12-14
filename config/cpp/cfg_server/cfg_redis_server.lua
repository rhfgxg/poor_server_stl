-- Redis 服务器配置文件

return {
    -- 服务器基本信息
    host = "0.0.0.0",       -- 监听地址
    port = 8003,            -- 监听端口

    -- 本地 Redis 配置
    local_redis = {
        host = "127.0.0.1",
        port = 6379,
        db = 0,
        password = nil,
        timeout_ms = 2000
    },

    -- Redis 集群配置
    cluster_redis = {
        -- 集群节点列表
        nodes = {
            "127.0.0.1:7000",
            "127.0.0.1:7001",
            "127.0.0.1:7002",
            "127.0.0.1:7003",
            "127.0.0.1:7004",
            "127.0.0.1:7005"
        },
        
        -- 单机模式（开发环境，如果没有集群）
        -- 取消注释下面一行以使用单机模式
        -- nodes = { "127.0.0.1:6379" },
        
        password = nil,
        timeout_ms = 2000
    },

    -- 日志配置
    log = {
        level = "debug",      -- trace, debug, info, warn, error, critical
        file_path = "logs/redis",
        max_size = 10485760,  -- 10MB
        max_files = 10
    }
}
