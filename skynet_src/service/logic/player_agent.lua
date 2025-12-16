-- Player Agent Service
-- 每个玩家一个实例，处理该玩家的所有游戏逻辑

local skynet = require "skynet"
local game_config = require "game_config"
local redis_bridge = require "redis_bridge"
local db_bridge = require "db_bridge"  -- 新增：数据库桥接

local player_id = ...
local player_data = {}
local online = false
local achievement_service

-- 数据同步相关
local last_redis_sync = 0
local last_mysql_sync = 0
local REDIS_SYNC_INTERVAL = 300  -- 5分钟同步一次到 Redis
local MYSQL_SYNC_INTERVAL = 1800  -- 30分钟同步一次到 MySQL
local data_dirty = false

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

-- 初始化玩家数据（三级加载）
local function init_player_data()
    skynet.error(string.format("Player %s: Loading data...", player_id))
    
    -- Level 1: 尝试从 Redis 加载
    local redis_data = redis_bridge.load_player_basic(player_id)
    
    if redis_data then
        skynet.error(string.format("Player %s: Data loaded from Redis cache", player_id))
        player_data = {
            player_id = player_id,
            level = redis_data.level,
            exp = redis_data.exp,
            gold = redis_data.gold,
            cards = {},
            decks = {},
            achievements = {
                progress = redis_bridge.load_player_achievements(player_id) or {},
                completed = {}
            },
            last_login = redis_data.last_login
        }
    else
        -- Level 2: Redis 没有，从 MySQL 加载
        skynet.error(string.format("Player %s: No Redis cache, loading from MySQL", player_id))
        
        local db_data = db_bridge.load_player_full(player_id)
        
        if db_data then
            skynet.error(string.format("Player %s: Data loaded from MySQL", player_id))
            player_data = {
                player_id = player_id,
                level = db_data.level,
                exp = db_data.exp,
                gold = db_data.gold,
                cards = db_data.cards or {},
                decks = {},
                achievements = db_data.achievements or {
                    progress = {},
                    completed = {}
                },
                last_login = db_data.last_login
            }
        else
            -- Level 3: 创建新玩家
            skynet.error(string.format("Player %s: New player created", player_id))
            player_data = {
                player_id = player_id,
                level = 1,
                exp = 0,
                gold = 1000,
                cards = {},
                decks = {},
                achievements = {
                    progress = {},
                    completed = {}
                },
                last_login = skynet.time()
            }
        end
        
        -- 立即缓存到 Redis
        save_to_redis()
    end
end

-- 保存到 Redis（缓存层）
local function save_to_redis()
    local ok = redis_bridge.save_player_basic(player_id, {
        level = player_data.level,
        exp = player_data.exp,
        gold = player_data.gold,
        last_login = player_data.last_login
    })
    
    if ok then
        -- 保存成就到 Redis
        for ach_id, progress in pairs(player_data.achievements.progress or {}) do
            redis_bridge.save_player_achievement(player_id, ach_id, progress)
        end
        
        skynet.error(string.format("Player %s: Data synced to Redis", player_id))
        last_redis_sync = skynet.time()
        return true
    else
        skynet.error(string.format("Player %s: Failed to sync to Redis", player_id))
        return false
    end
end

-- 保存到 MySQL（持久化层）
local function save_to_mysql()
    local ok = db_bridge.save_player_full(player_id, player_data)
    
    if ok then
        skynet.error(string.format("Player %s: Data persisted to MySQL", player_id))
        last_mysql_sync = skynet.time()
        data_dirty = false
        return true
    else
        skynet.error(string.format("Player %s: Failed to persist to MySQL", player_id))
        return false
    end
end

-- 定时同步（Redis 和 MySQL）
local function auto_sync()
    while online do
        skynet.sleep(100)  -- 每秒检查一次
        
        local now = skynet.time()
        
        -- 定时同步到 Redis（每 5 分钟）
        if data_dirty and (now - last_redis_sync >= REDIS_SYNC_INTERVAL) then
            save_to_redis()
        end
        
        -- 定时持久化到 MySQL（每 30 分钟）
        if data_dirty and (now - last_mysql_sync >= MYSQL_SYNC_INTERVAL) then
            save_to_mysql()
        end
    end
end

-- 标记数据已修改
local function mark_dirty()
    data_dirty = true
end

-- 处理玩家动作
function CMD.action(action_data)
    skynet.error(string.format("Player %s action: %s", player_id, action_data))
    
    local response = "action_ok:" .. action_data

    if action_data == "collect_gold" then
        player_data.gold = player_data.gold + 100
        mark_dirty()
        response = "action_ok:collect_gold|gold:" .. player_data.gold
    end

    if action_data == "complete_match" then
        player_data.exp = player_data.exp + 50
        mark_dirty()
        
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
    mark_dirty()
    
    skynet.error(string.format("Player %s online", player_id))
    
    -- 启动定时同步
    skynet.fork(auto_sync)
end

-- 玩家下线
function CMD.offline()
    online = false
    
    -- 立即保存所有数据（三级同步）
    skynet.error(string.format("Player %s offline, saving data...", player_id))
    
    -- 1. 保存到 Redis
    save_to_redis()
    
    -- 2. 保存到 MySQL
    save_to_mysql()
    
    skynet.error(string.format("Player %s offline complete", player_id))
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
    
    -- 初始化数据库连接
    redis_bridge.init()
    db_bridge.init()
    
    -- 初始化玩家数据
    init_player_data()
    CMD.online()
    
    skynet.error(string.format("Player Agent started for: %s", player_id))
end)
