-- Hearthstone Manager Service
-- 管理炉石传说相关的游戏逻辑

local skynet = require "skynet"

local CMD = {}
local battles = {}  -- battle_id -> battle_service
local matchmaking_queue = {}  -- 匹配队列

-- 创建战斗
function CMD.create_battle(player1_id, player2_id)
    local battle_id = string.format("battle_%s_%s_%d", player1_id, player2_id, skynet.time())
    
    -- 创建战斗服务
    local battle_service = skynet.newservice("logic/hearthstone_battle", battle_id, player1_id, player2_id)
    battles[battle_id] = battle_service
    
    skynet.error(string.format("Created battle: %s", battle_id))
    return battle_id, battle_service
end

-- 玩家进入匹配队列
function CMD.join_matchmaking(player_id)
    table.insert(matchmaking_queue, player_id)
    skynet.error(string.format("Player %s joined matchmaking queue", player_id))
    
    -- 简化：如果队列有 2 个玩家，立即匹配
    if #matchmaking_queue >= 2 then
        local p1 = table.remove(matchmaking_queue, 1)
        local p2 = table.remove(matchmaking_queue, 1)
        
        local battle_id, battle_service = CMD.create_battle(p1, p2)
        
        -- 通知双方玩家
        -- TODO: 通过 cpp_gateway 发送消息给玩家
        
        return battle_id
    end
    
    return nil
end

-- 玩家离开匹配队列
function CMD.leave_matchmaking(player_id)
    for i, pid in ipairs(matchmaking_queue) do
        if pid == player_id then
            table.remove(matchmaking_queue, i)
            skynet.error(string.format("Player %s left matchmaking queue", player_id))
            return true
        end
    end
    return false
end

-- 获取战斗信息
function CMD.get_battle(battle_id)
    return battles[battle_id]
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
    
    skynet.error("Hearthstone Manager started")
end)
