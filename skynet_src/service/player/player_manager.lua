-- Player Manager Service
-- 管理所有玩家的生命周期

local skynet = require "skynet"

local CMD = {}
local players = {}  -- player_id -> player_service

-- 创建或获取玩家服务
function CMD.get_or_create_player(player_id)
    local player_service = players[player_id]
    
    if not player_service then
        -- 创建新的玩家服务
        player_service = skynet.newservice("logic/player_agent", player_id)
        players[player_id] = player_service
        
        skynet.error(string.format("Created new player service for %s: %s", 
            player_id, skynet.address(player_service)))
    else
        skynet.error(string.format("Player %s already has service: %s", 
            player_id, skynet.address(player_service)))
    end
    
    return player_service
end

-- 玩家离线
function CMD.player_offline(player_id)
    local player_service = players[player_id]
    if player_service then
        -- 通知玩家服务下线
        skynet.send(player_service, "lua", "offline")
        
        -- 可选：延迟杀死服务（给玩家数据保存时间）
        skynet.timeout(300, function()  -- 30秒后
            if players[player_id] == player_service then
                skynet.kill(player_service)
                players[player_id] = nil
                skynet.error("Player service killed:", player_id)
            end
        end)
    end
end

-- 获取玩家服务
function CMD.get_player(player_id)
    return players[player_id]
end

-- 获取在线玩家列表
function CMD.get_online_players()
    local online = {}
    for player_id, _ in pairs(players) do
        table.insert(online, player_id)
    end
    return online
end

-- 启动服务
skynet.start(function()
    skynet.dispatch("lua", function(session, source, cmd, ...)
        local f = CMD[cmd]
        if f then
            if session == 0 then
                f(...)
            else
                skynet.ret(skynet.pack(f(...)))
            end
        else
            skynet.error("Unknown command:", cmd)
        end
    end)
    
    skynet.error("Player Manager started")
end)
