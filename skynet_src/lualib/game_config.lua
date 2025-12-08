-- 业务配置文件
-- 此文件可以在 Skynet 服务中 require "game_config" 使用

local config = {}

-- 数据库配置
config.db = {
    host = "127.0.0.1",
    port = 3306,
    user = "root",
    password = "159357",
    database = "poor_hearthstone",
    charset = "utf8mb4"
}

-- Redis 配置
config.redis = {
    host = "127.0.0.1",
    port = 6379,
    db = 0
}

-- C++ 服务配置
config.cpp_services = {
    db_server = "127.0.0.1:50052",
    central_server = "127.0.0.1:50051",
    gateway_server = "127.0.0.1:50053"
}

-- 游戏配置
config.game = {
    max_players_per_battle = 2,
    battle_timeout = 600,  -- 10 分钟
    matchmaking_timeout = 60  -- 1 分钟
}

return config
