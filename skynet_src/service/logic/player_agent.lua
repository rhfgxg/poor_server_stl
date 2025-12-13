-- Player Agent Service
-- 每个玩家一个实例，处理该玩家的所有游戏逻辑

local skynet = require "skynet"
local game_config = require "game_config"

local player_id = ...  -- 启动参数：玩家ID
local player_data = {}
local online = false
local achievement_service

local CMD = {}

local function get_achievement_service()
    if not achievement_service then
        achievement_service = skynet.queryservice("logic/achievement_service")
    end
    return achievement_service
end

local function update_achievements(event_type, delta)
    local service = get_achievement_service()
    if not service then
        return nil
    end

    local ok, result = pcall(skynet.call, service, "lua", "update_progress", player_id, event_type, delta or 1)
    if not ok then
        skynet.error(string.format("Player %s achievement update failed: %s", player_id, result))
        return nil
    end

    if result then
        player_data.achievements.progress = result.state or player_data.achievements.progress
        player_data.achievements.completed = player_data.achievements.completed or {}
        for _, entry in ipairs(result.completed or {}) do
            player_data.achievements.completed[entry.id] = true
        end
    end

    return result
end

-- 初始化玩家数据
local function init_player_data()
    -- TODO: 从数据库加载玩家数据
    -- 可以使用 game_config.db 中的配置
    player_data = {
        player_id = player_id,
        level = 1,
        exp = 0,
        gold = 1000,
        cards = {},  -- 拥有的卡牌
        decks = {},  -- 卡组
        achievements = {
            progress = {},
            completed = {}
        },
        last_login = skynet.time()
    }
    
    skynet.error(string.format("Player %s data initialized", player_id))
end

-- 保存玩家数据
local function save_player_data()
    -- TODO: 保存到数据库
    skynet.error(string.format("Player %s data saved", player_id))
end

-- 处理玩家动作
function CMD.action(action_data)
    skynet.error(string.format("Player %s action: %s", player_id, action_data))
    
    local response = "action_ok:" .. action_data

    if action_data == "complete_match" then
        local result = update_achievements("complete_match", 1)
        if result and result.updated and #result.completed > 0 then
            local unlocked = {}
            for _, entry in ipairs(result.completed) do
                table.insert(unlocked, tostring(entry.id))
            end
            response = response .. "|achievement:" .. table.concat(unlocked, ",")
        end
    end
    
    return response
end

-- 玩家上线
function CMD.online()
    online = true
    player_data.last_login = skynet.time()
    skynet.error(string.format("Player %s online", player_id))
end

-- 玩家下线
function CMD.offline()
    online = false
    save_player_data()
    skynet.error(string.format("Player %s offline", player_id))
end

-- 获取玩家数据
function CMD.get_data()
    return player_data
end

function CMD.get_achievements()
    return player_data.achievements
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
    
    -- 初始化
    init_player_data()
    CMD.online()
    
    skynet.error(string.format("Player Agent started for: %s", player_id))
end)
