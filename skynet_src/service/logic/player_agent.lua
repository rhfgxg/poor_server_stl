--[[
    Player Agent Service
    每个玩家一个实例，处理该玩家的所有游戏逻辑
    
    使用模块：
    - redis.lua : 缓存层
    - db.lua    : 持久化层
]]

local skynet = require "skynet"
local game_config = require "game_config"
local redis = require "redis"
local db = require "db"

local player_id = ...
local player_data = {}
local online = false
local achievement_service

-- 数据同步配置
local last_redis_sync = 0
local last_mysql_sync = 0
local REDIS_SYNC_INTERVAL = 300   -- 5分钟同步到 Redis
local MYSQL_SYNC_INTERVAL = 1800  -- 30分钟同步到 MySQL
local data_dirty = false

local CMD = {}

-- ==================== 辅助函数 ====================

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

    local ok, result = pcall(skynet.call, service, "lua", "update_progress", 
        player_id, event_type, delta or 1)
    
    if not ok then
        skynet.error(string.format("[Player %s] Achievement update failed: %s", player_id, result))
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

-- 标记数据已修改
local function mark_dirty()
    data_dirty = true
end

-- ==================== 数据加载/保存 ====================

-- 保存到 Redis（缓存层）
local function save_to_redis()
    -- 保存基础数据
    local ok = redis.player.save_basic(player_id, {
        player_id = player_data.player_id,
        nickname = player_data.nickname,
        gold = player_data.gold,
        arcane_dust = player_data.arcane_dust,
        last_login = player_data.last_login,
    })
    
    if ok then
        -- 保存成就进度
        for ach_id, progress in pairs(player_data.achievements.progress or {}) do
            redis.player.save_achievement(player_id, ach_id, progress)
        end
        
        skynet.error(string.format("[Player %s] Synced to Redis", player_id))
        last_redis_sync = skynet.time()
        return true
    else
        skynet.error(string.format("[Player %s] Failed to sync to Redis", player_id))
        return false
    end
end

-- 保存到 MySQL（持久化层）
local function save_to_mysql()
    local ok = db.player.save_full(player_id, player_data)
    
    if ok then
        skynet.error(string.format("[Player %s] Persisted to MySQL", player_id))
        last_mysql_sync = skynet.time()
        data_dirty = false
        return true
    else
        skynet.error(string.format("[Player %s] Failed to persist to MySQL", player_id))
        return false
    end
end

-- 初始化玩家数据（三级加载：Redis -> MySQL -> 新建）
local function init_player_data()
    skynet.error(string.format("[Player %s] Loading data...", player_id))
    
    -- Level 1: 尝试从 Redis 加载
    local redis_data = redis.player.load_basic(player_id)
    
    if redis_data and redis_data.player_id then
        skynet.error(string.format("[Player %s] Loaded from Redis cache", player_id))
        player_data = {
            player_id = player_id,
            nickname = redis_data.nickname or ("Player_" .. player_id),
            gold = redis_data.gold or 0,
            arcane_dust = redis_data.arcane_dust or 0,
            cards = {},
            decks = {},
            achievements = {
                progress = redis.player.load_achievements(player_id) or {},
                completed = {},
            },
            last_login = redis_data.last_login or os.time(),
        }
        return
    end
    
    -- Level 2: Redis 没有，从 MySQL 加载
    skynet.error(string.format("[Player %s] No Redis cache, loading from MySQL", player_id))
    local db_data = db.player.load_full(player_id)
    
    if db_data then
        skynet.error(string.format("[Player %s] Loaded from MySQL", player_id))
        player_data = {
            player_id = player_id,
            nickname = db_data.nickname or ("Player_" .. player_id),
            gold = db_data.gold or 0,
            arcane_dust = db_data.arcane_dust or 0,
            cards = db_data.cards or {},
            decks = db_data.decks or {},
            achievements = db_data.achievements or {
                progress = {},
                completed = {},
            },
            last_login = db_data.last_login or os.time(),
        }
        
        -- 缓存到 Redis
        save_to_redis()
        return
    end
    
    -- Level 3: 创建新玩家
    skynet.error(string.format("[Player %s] New player created", player_id))
    player_data = {
        player_id = player_id,
        nickname = "Player_" .. player_id,
        gold = 1000,        -- 初始金币
        arcane_dust = 0,
        cards = {},
        decks = {},
        achievements = {
            progress = {},
            completed = {},
        },
        last_login = os.time(),
    }
    
    -- 保存到数据库和缓存
    db.player.create(player_id, player_id, player_data.nickname)
    save_to_redis()
end

-- 定时同步协程
local function auto_sync()
    while online do
        skynet.sleep(100)  -- 每秒检查
        
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

-- ==================== 服务命令 ====================

-- 处理玩家动作
function CMD.action(action_type, action_data)
    skynet.error(string.format("[Player %s] Action: %s", player_id, action_type))
    
    local response = "action_ok:" .. (action_type or "unknown")

    if action_type == "collect_gold" then
        player_data.gold = player_data.gold + 100
        mark_dirty()
        response = "action_ok:collect_gold|gold:" .. player_data.gold
        
    elseif action_type == "complete_match" then
        -- 完成一场对战，获得经验和金币
        player_data.gold = player_data.gold + 50
        mark_dirty()
        
        -- 更新成就进度
        local result = update_achievements("complete_match", 1)
        if result and result.completed and #result.completed > 0 then
            local unlocked = {}
            for _, entry in ipairs(result.completed) do
                table.insert(unlocked, tostring(entry.id))
            end
            response = response .. "|achievement:" .. table.concat(unlocked, ",")
        end
        
    elseif action_type == "win_match" then
        -- 赢得对战
        player_data.gold = player_data.gold + 100
        mark_dirty()
        
        update_achievements("win_match", 1)
        response = "action_ok:win_match|gold:" .. player_data.gold
        
    elseif action_type == "buy_card" then
        -- 购买卡牌
        local cost = 100
        if player_data.gold >= cost then
            player_data.gold = player_data.gold - cost
            mark_dirty()
            response = "action_ok:buy_card|gold:" .. player_data.gold
        else
            response = "action_failed:not_enough_gold"
        end
    end
    
    return response
end

-- 玩家上线
function CMD.online()
    online = true
    player_data.last_login = os.time()
    mark_dirty()
    
    skynet.error(string.format("[Player %s] Online", player_id))
    
    -- 启动定时同步
    skynet.fork(auto_sync)
end

-- 玩家下线
function CMD.offline()
    online = false
    
    skynet.error(string.format("[Player %s] Offline, saving data...", player_id))
    
    -- 立即保存所有数据
    save_to_redis()
    save_to_mysql()
    
    skynet.error(string.format("[Player %s] Offline complete", player_id))
end

-- 获取玩家数据
function CMD.get_data()
    return player_data
end

-- 获取成就数据
function CMD.get_achievements()
    return player_data.achievements
end

-- 更新金币
function CMD.add_gold(amount)
    player_data.gold = player_data.gold + amount
    mark_dirty()
    return player_data.gold
end

-- 更新奥术之尘
function CMD.add_arcane_dust(amount)
    player_data.arcane_dust = player_data.arcane_dust + amount
    mark_dirty()
    return player_data.arcane_dust
end

-- ==================== 服务启动 ====================

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
            skynet.error("[Player " .. player_id .. "] Unknown command:", cmd)
        end
    end)
    
    -- 初始化模块（在服务中只需确保已初始化）
    -- redis.init() 和 db.init() 应该在 main.lua 中调用
    -- 这里可以跳过，因为 main.lua 已初始化
    
    -- 初始化玩家数据
    init_player_data()
    CMD.online()
    
    skynet.error(string.format("[Player %s] Agent started", player_id))
end)
