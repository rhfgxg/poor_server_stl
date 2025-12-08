-- Hearthstone Battle Service
-- 管理单场战斗的逻辑

local skynet = require "skynet"

local battle_id, player1_id, player2_id = ...

local battle_state = {
    battle_id = battle_id,
    player1 = player1_id,
    player2 = player2_id,
    current_turn = 1,
    current_player = player1_id,
    started = false,
    finished = false
}

local CMD = {}

-- 开始战斗
function CMD.start()
    battle_state.started = true
    skynet.error(string.format("Battle %s started: %s vs %s", battle_id, player1_id, player2_id))
    
    -- TODO: 初始化战斗状态、抽牌等
end

-- 玩家行动
function CMD.player_action(player_id, action_type, action_data)
    if battle_state.finished then
        return false, "battle finished"
    end
    
    if battle_state.current_player ~= player_id then
        return false, "not your turn"
    end
    
    -- TODO: 处理行动（出牌、攻击、结束回合等）
    skynet.error(string.format("Player %s action in battle %s: %s", player_id, battle_id, action_type))
    
    return true, "action ok"
end

-- 结束回合
function CMD.end_turn(player_id)
    if battle_state.current_player ~= player_id then
        return false, "not your turn"
    end
    
    -- 切换玩家
    battle_state.current_player = (battle_state.current_player == player1_id) and player2_id or player1_id
    battle_state.current_turn = battle_state.current_turn + 1
    
    skynet.error(string.format("Battle %s turn %d, current player: %s", 
        battle_id, battle_state.current_turn, battle_state.current_player))
    
    return true
end

-- 获取战斗状态
function CMD.get_state()
    return battle_state
end

-- 结束战斗
function CMD.finish(winner_id)
    battle_state.finished = true
    battle_state.winner = winner_id
    
    skynet.error(string.format("Battle %s finished, winner: %s", battle_id, winner_id))
    
    -- TODO: 结算奖励、保存战斗记录等
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
    
    skynet.error(string.format("Hearthstone Battle service started: %s", battle_id))
    
    -- 自动开始战斗
    CMD.start()
end)
