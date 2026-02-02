--[[
    玩家缓存业务模块
    
    提供玩家相关的 Redis 缓存操作封装
    从 redis.lua 分离，保持工具类的纯净
    
    使用方式：
        local player_cache = require "player_cache"
        
        -- 保存玩家数据
        player_cache.save_basic(player_id, data)
        
        -- 加载玩家数据
        local data = player_cache.load_basic(player_id)
        
        -- 会话管理
        player_cache.set_session(player_id, session_data)
        local session = player_cache.get_session(player_id)
]]

local skynet = require "skynet"
local redis = require "redis"

local M = {}

-- ==================== 配置 ====================

local PLAYER_CACHE_TTL = 1800  -- 30分钟
local SESSION_TTL = 3600       -- 1小时

-- ==================== 玩家基础数据 ====================

--- 保存玩家基础数据到缓存
--- @param player_id string 玩家ID
--- @param data table 玩家数据
--- @return boolean 是否成功
function M.save_basic(player_id, data)
    local key = redis.make_key("player", player_id, "basic")
    
    -- 简单 JSON 序列化
    local json = string.format(
        '{"player_id":"%s","nickname":"%s","level":%d,"exp":%d,"gold":%d,"arcane_dust":%d,"last_login":%d}',
        tostring(data.player_id or player_id),
        tostring(data.nickname or ""),
        data.level or 1,
        data.exp or 0,
        data.gold or 0,
        data.arcane_dust or 0,
        data.last_login or os.time()
    )
    
    return redis.setex(key, json, PLAYER_CACHE_TTL)
end

--- 从缓存加载玩家基础数据
--- @param player_id string 玩家ID
--- @return table|nil 玩家数据
function M.load_basic(player_id)
    local key = redis.make_key("player", player_id, "basic")
    local json = redis.get(key)
    
    if not json then
        return nil
    end
    
    -- 简单 JSON 解析
    local data = {}
    data.player_id = json:match('"player_id":"([^"]*)"') or player_id
    data.nickname = json:match('"nickname":"([^"]*)"') or ""
    data.level = tonumber(json:match('"level":(%d+)')) or 1
    data.exp = tonumber(json:match('"exp":(%d+)')) or 0
    data.gold = tonumber(json:match('"gold":(%d+)')) or 0
    data.arcane_dust = tonumber(json:match('"arcane_dust":(%d+)')) or 0
    data.last_login = tonumber(json:match('"last_login":(%d+)')) or 0
    
    return data
end

--- 删除玩家所有缓存
--- @param player_id string 玩家ID
--- @return boolean 是否成功
function M.clear(player_id)
    local keys = {
        redis.make_key("player", player_id, "basic"),
        redis.make_key("player", player_id, "achievements"),
        redis.make_key("player", player_id, "gold"),
        redis.make_key("session", player_id),
    }
    
    for _, key in ipairs(keys) do
        redis.del(key)
    end
    return true
end

-- ==================== 成就数据 ====================

--- 保存成就进度
--- @param player_id string 玩家ID
--- @param achievement_id string 成就ID
--- @param progress number 进度值
--- @return boolean 是否成功
function M.save_achievement(player_id, achievement_id, progress)
    local key = redis.make_key("player", player_id, "achievements")
    return redis.hset(key, achievement_id, progress)
end

--- 加载所有成就进度
--- @param player_id string 玩家ID
--- @return table 成就进度表 {achievement_id = progress, ...}
function M.load_achievements(player_id)
    local key = redis.make_key("player", player_id, "achievements")
    local hash = redis.hgetall(key)
    
    local achievements = {}
    for id, progress in pairs(hash) do
        achievements[id] = tonumber(progress) or 0
    end
    return achievements
end

--- 批量保存成就进度
--- @param player_id string 玩家ID
--- @param achievements table 成就进度表 {achievement_id = progress, ...}
--- @return boolean 是否成功
function M.save_achievements(player_id, achievements)
    local key = redis.make_key("player", player_id, "achievements")
    return redis.hmset(key, achievements)
end

-- ==================== 金币操作 ====================

--- 增加金币（原子操作）
--- @param player_id string 玩家ID
--- @param amount number 增加数量
--- @return number 新的金币数
function M.add_gold(player_id, amount)
    local key = redis.make_key("player", player_id, "gold")
    return redis.incrby(key, amount)
end

--- 获取金币数
--- @param player_id string 玩家ID
--- @return number 金币数
function M.get_gold(player_id)
    local key = redis.make_key("player", player_id, "gold")
    local value = redis.get(key)
    return tonumber(value) or 0
end

--- 设置金币数
--- @param player_id string 玩家ID
--- @param amount number 金币数
--- @return boolean 是否成功
function M.set_gold(player_id, amount)
    local key = redis.make_key("player", player_id, "gold")
    return redis.set(key, tostring(amount))
end

-- ==================== 会话管理 ====================

--- 设置玩家会话
--- @param player_id string 玩家ID
--- @param session_data table 会话数据
--- @param ttl number|nil 过期时间（秒）
--- @return boolean 是否成功
function M.set_session(player_id, session_data, ttl)
    local key = redis.make_key("session", player_id)
    
    local json = string.format(
        '{"player_id":"%s","login_time":%d,"last_active":%d,"gateway":"%s"}',
        player_id,
        session_data.login_time or os.time(),
        session_data.last_active or os.time(),
        tostring(session_data.gateway or "")
    )
    
    return redis.setex(key, json, ttl or SESSION_TTL)
end

--- 获取玩家会话
--- @param player_id string 玩家ID
--- @return table|nil 会话数据
function M.get_session(player_id)
    local key = redis.make_key("session", player_id)
    local json = redis.get(key)
    
    if not json then
        return nil
    end
    
    return {
        player_id = json:match('"player_id":"([^"]*)"'),
        login_time = tonumber(json:match('"login_time":(%d+)')) or 0,
        last_active = tonumber(json:match('"last_active":(%d+)')) or 0,
        gateway = json:match('"gateway":"([^"]*)"') or "",
    }
end

--- 刷新会话活跃时间
--- @param player_id string 玩家ID
--- @param ttl number|nil 过期时间（秒）
--- @return boolean 是否成功
function M.refresh_session(player_id, ttl)
    local key = redis.make_key("session", player_id)
    
    -- 更新 last_active 并刷新过期时间
    local session = M.get_session(player_id)
    if session then
        session.last_active = os.time()
        return M.set_session(player_id, session, ttl)
    end
    
    return redis.expire(key, ttl or SESSION_TTL)
end

--- 删除会话
--- @param player_id string 玩家ID
--- @return boolean 是否成功
function M.delete_session(player_id)
    local key = redis.make_key("session", player_id)
    return redis.del(key)
end

--- 检查会话是否存在
--- @param player_id string 玩家ID
--- @return boolean 是否存在
function M.has_session(player_id)
    local key = redis.make_key("session", player_id)
    return redis.exists(key)
end

return M
