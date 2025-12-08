-- Gateway Skynet 配置
-- Gateway 如何路由消息到 Skynet

local skynet_routes = {
    -- Skynet Logic Server 配置
    logic = {
        host = "127.0.0.1",
        port = 8888,  -- cpp_gateway 端口
        
        -- 哪些消息类型路由到 Skynet
        message_types = {
            "ENTER_GAME",
            "LEAVE_GAME",
            "PLAYER_ACTION",
            "GAME_STATE",
            "BATTLE_ACTION",
            "MATCHMAKING",
        }
    }
}

return skynet_routes
