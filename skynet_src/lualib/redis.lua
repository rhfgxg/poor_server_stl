--[[
    Redis 封装模块
    提供简洁的 Redis 操作接口
    
    使用方式：
        local redis = require "redis"
        
        -- 初始化（启动时调用一次）
        redis.init()
        
        -- 基本操作
        redis.set("key", "value")
        local value = redis.get("key")
        redis.del("key")
        
        -- 带过期时间
        redis.setex("key", "value", 3600)  -- 1小时过期
        
        -- Hash 操作
        redis.hset("hash_key", "field", "value")
        local value = redis.hget("hash_key", "field")
        local all = redis.hgetall("hash_key")
        
        -- 玩家数据快捷接口
        redis.player.save_basic(player_id, data)
        local data = redis.player.load_basic(player_id)
]]

local skynet = require "skynet"

local M = {}
M.player = {}  -- 玩家数据快捷接口

-- ==================== 配置 ====================

local config = {
    host = "127.0.0.1",
    port = 6379,
    db = 0,
    auth = nil,
    timeout = 5000,
}

-- ==================== 连接管理 ====================

local redis_conn = nil
local using_skynet_redis = false
local fallback_storage = nil  -- 内存降级存储

-- 尝试加载 skynet.db.redis
local function try_load_skynet_redis()
    local ok, redis_module = pcall(require, "skynet.db.redis")
    if ok then
        return redis_module
    end
    skynet.error("[Redis] skynet.db.redis not available:", redis_module)
    return nil
end

-- 初始化内存降级模式
local function init_fallback()
    skynet.error("[Redis] Using in-memory fallback mode")
    skynet.error("[Redis] WARNING: Data will NOT persist!")
    
    fallback_storage = {
        data = {},
        expiry = {},
        hash = {},
    }
    return true
end

-- 检查并清理过期 key
local function check_expiry(key)
    if fallback_storage and fallback_storage.expiry[key] then
        if os.time() > fallback_storage.expiry[key] then
            fallback_storage.data[key] = nil
            fallback_storage.expiry[key] = nil
            return true  -- 已过期
        end
    end
    return false
end

-- ==================== 初始化接口 ====================

--- 初始化 Redis 连接
--- @param cfg table|nil 配置（可选）
--- @return boolean 是否成功
function M.init(cfg)
    if redis_conn or fallback_storage then
        skynet.error("[Redis] Already initialized")
        return true
    end
    
    -- 合并配置
    if cfg then
        for k, v in pairs(cfg) do
            config[k] = v
        end
    end
    
    skynet.error("[Redis] Initializing...")
    skynet.error("[Redis] Target:", config.host .. ":" .. config.port)
    
    -- 尝试使用 skynet.db.redis
    local redis_module = try_load_skynet_redis()
    if redis_module then
        local ok, conn = pcall(redis_module.connect, {
            host = config.host,
            port = config.port,
            db = config.db,
            auth = config.auth,
        })
        
        if ok and conn then
            redis_conn = conn
            using_skynet_redis = true
            skynet.error("[Redis] Connected via skynet.db.redis")
            return true
        else
            skynet.error("[Redis] Connection failed:", tostring(conn))
        end
    end
    
    -- 降级到内存模式
    return init_fallback()
end

--- 关闭 Redis 连接
function M.close()
    if redis_conn and using_skynet_redis then
        pcall(function() redis_conn:disconnect() end)
        redis_conn = nil
    end
    fallback_storage = nil
    skynet.error("[Redis] Connection closed")
end

--- 获取连接状态信息
function M.info()
    return {
        connected = redis_conn ~= nil or fallback_storage ~= nil,
        using_redis = using_skynet_redis,
        using_fallback = fallback_storage ~= nil,
        config = {
            host = config.host,
            port = config.port,
            db = config.db,
        }
    }
end

-- ==================== 基本命令 ====================

--- SET key value
function M.set(key, value)
    if using_skynet_redis and redis_conn then
        local ok, err = pcall(function()
            redis_conn:set(key, tostring(value))
        end)
        return ok
    elseif fallback_storage then
        fallback_storage.data[key] = tostring(value)
        return true
    end
    return false
end

--- SETEX key seconds value
function M.setex(key, value, ttl_seconds)
    if using_skynet_redis and redis_conn then
        local ok, err = pcall(function()
            redis_conn:setex(key, ttl_seconds, tostring(value))
        end)
        return ok
    elseif fallback_storage then
        fallback_storage.data[key] = tostring(value)
        fallback_storage.expiry[key] = os.time() + ttl_seconds
        return true
    end
    return false
end

--- GET key
function M.get(key)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:get(key)
        end)
        if ok then
            return result
        end
        return nil
    elseif fallback_storage then
        if check_expiry(key) then
            return nil
        end
        return fallback_storage.data[key]
    end
    return nil
end

--- DEL key
function M.del(key)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:del(key)
        end)
        return ok and result > 0
    elseif fallback_storage then
        if fallback_storage.data[key] then
            fallback_storage.data[key] = nil
            fallback_storage.expiry[key] = nil
            return true
        end
        return false
    end
    return false
end

--- EXISTS key
function M.exists(key)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:exists(key)
        end)
        return ok and result > 0
    elseif fallback_storage then
        if check_expiry(key) then
            return false
        end
        return fallback_storage.data[key] ~= nil
    end
    return false
end

--- EXPIRE key seconds
function M.expire(key, ttl_seconds)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:expire(key, ttl_seconds)
        end)
        return ok and result > 0
    elseif fallback_storage then
        if fallback_storage.data[key] then
            fallback_storage.expiry[key] = os.time() + ttl_seconds
            return true
        end
        return false
    end
    return false
end

--- TTL key
function M.ttl(key)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:ttl(key)
        end)
        if ok then
            return result
        end
        return -3
    elseif fallback_storage then
        if not fallback_storage.data[key] then
            return -2
        end
        if fallback_storage.expiry[key] then
            local remaining = fallback_storage.expiry[key] - os.time()
            return remaining > 0 and remaining or -2
        end
        return -1
    end
    return -3
end

--- INCR key
function M.incr(key)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:incr(key)
        end)
        if ok then
            return result
        end
        return nil
    elseif fallback_storage then
        local value = tonumber(fallback_storage.data[key]) or 0
        value = value + 1
        fallback_storage.data[key] = tostring(value)
        return value
    end
    return nil
end

--- INCRBY key increment
function M.incrby(key, increment)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:incrby(key, increment)
        end)
        if ok then
            return result
        end
        return nil
    elseif fallback_storage then
        local value = tonumber(fallback_storage.data[key]) or 0
        value = value + increment
        fallback_storage.data[key] = tostring(value)
        return value
    end
    return nil
end

-- ==================== Hash 命令 ====================

--- HSET key field value
function M.hset(key, field, value)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:hset(key, field, tostring(value))
        end)
        return ok
    elseif fallback_storage then
        if not fallback_storage.hash[key] then
            fallback_storage.hash[key] = {}
        end
        fallback_storage.hash[key][field] = tostring(value)
        return true
    end
    return false
end

--- HGET key field
function M.hget(key, field)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:hget(key, field)
        end)
        if ok then
            return result
        end
        return nil
    elseif fallback_storage then
        if fallback_storage.hash[key] then
            return fallback_storage.hash[key][field]
        end
        return nil
    end
    return nil
end

--- HDEL key field
function M.hdel(key, field)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:hdel(key, field)
        end)
        return ok and result > 0
    elseif fallback_storage then
        if fallback_storage.hash[key] and fallback_storage.hash[key][field] then
            fallback_storage.hash[key][field] = nil
            return true
        end
        return false
    end
    return false
end

--- HGETALL key
function M.hgetall(key)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:hgetall(key)
        end)
        if ok and result then
            -- skynet.db.redis 返回数组格式，需要转换
            local hash = {}
            for i = 1, #result, 2 do
                hash[result[i]] = result[i + 1]
            end
            return hash
        end
        return {}
    elseif fallback_storage then
        return fallback_storage.hash[key] or {}
    end
    return {}
end

--- HMSET key field1 value1 field2 value2 ...
function M.hmset(key, fields)
    if using_skynet_redis and redis_conn then
        local args = {}
        for field, value in pairs(fields) do
            table.insert(args, field)
            table.insert(args, tostring(value))
        end
        local ok = pcall(function()
            redis_conn:hmset(key, table.unpack(args))
        end)
        return ok
    elseif fallback_storage then
        if not fallback_storage.hash[key] then
            fallback_storage.hash[key] = {}
        end
        for field, value in pairs(fields) do
            fallback_storage.hash[key][field] = tostring(value)
        end
        return true
    end
    return false
end

--- HINCRBY key field increment
function M.hincrby(key, field, increment)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:hincrby(key, field, increment)
        end)
        if ok then
            return result
        end
        return nil
    elseif fallback_storage then
        if not fallback_storage.hash[key] then
            fallback_storage.hash[key] = {}
        end
        local value = tonumber(fallback_storage.hash[key][field]) or 0
        value = value + increment
        fallback_storage.hash[key][field] = tostring(value)
        return value
    end
    return nil
end

-- ==================== List 命令 ====================

--- LPUSH key value
function M.lpush(key, value)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:lpush(key, tostring(value))
        end)
        if ok then
            return result
        end
        return nil
    elseif fallback_storage then
        if not fallback_storage.data[key] then
            fallback_storage.data[key] = {}
        end
        table.insert(fallback_storage.data[key], 1, tostring(value))
        return #fallback_storage.data[key]
    end
    return nil
end

--- RPUSH key value
function M.rpush(key, value)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:rpush(key, tostring(value))
        end)
        if ok then
            return result
        end
        return nil
    elseif fallback_storage then
        if not fallback_storage.data[key] then
            fallback_storage.data[key] = {}
        end
        table.insert(fallback_storage.data[key], tostring(value))
        return #fallback_storage.data[key]
    end
    return nil
end

--- LPOP key
function M.lpop(key)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:lpop(key)
        end)
        if ok then
            return result
        end
        return nil
    elseif fallback_storage then
        if fallback_storage.data[key] and type(fallback_storage.data[key]) == "table" then
            return table.remove(fallback_storage.data[key], 1)
        end
        return nil
    end
    return nil
end

--- LRANGE key start stop
function M.lrange(key, start, stop)
    if using_skynet_redis and redis_conn then
        local ok, result = pcall(function()
            return redis_conn:lrange(key, start, stop)
        end)
        if ok then
            return result
        end
        return {}
    elseif fallback_storage then
        if fallback_storage.data[key] and type(fallback_storage.data[key]) == "table" then
            local result = {}
            local list = fallback_storage.data[key]
            local len = #list
            
            -- Lua 索引从 1 开始，Redis 从 0 开始
            local s = start + 1
            local e = stop < 0 and (len + stop + 1) or (stop + 1)
            e = math.min(e, len)
            
            for i = s, e do
                table.insert(result, list[i])
            end
            return result
        end
        return {}
    end
    return {}
end

-- ==================== 工具函数 ====================

--- 构造 key（命名空间风格）
--- @param namespace string 命名空间
--- @param ... 其他部分
--- @return string
function M.make_key(namespace, ...)
    local parts = {namespace}
    for _, part in ipairs({...}) do
        table.insert(parts, tostring(part))
    end
    return table.concat(parts, ":")
end

-- ==================== 玩家数据快捷接口 ====================

local PLAYER_CACHE_TTL = 1800  -- 30分钟

--- 保存玩家基础数据到缓存
function M.player.save_basic(player_id, data)
    local key = M.make_key("player", player_id, "basic")
    
    -- 简单 JSON 序列化
    local json = string.format(
        '{"player_id":"%s","nickname":"%s","gold":%d,"arcane_dust":%d,"last_login":%d}',
        tostring(data.player_id or player_id),
        tostring(data.nickname or ""),
        data.gold or 0,
        data.arcane_dust or 0,
        data.last_login or os.time()
    )
    
    return M.setex(key, json, PLAYER_CACHE_TTL)
end

--- 从缓存加载玩家基础数据
function M.player.load_basic(player_id)
    local key = M.make_key("player", player_id, "basic")
    local json = M.get(key)
    
    if not json then
        return nil
    end
    
    -- 简单 JSON 解析
    local data = {}
    data.player_id = json:match('"player_id":"([^"]*)"') or player_id
    data.nickname = json:match('"nickname":"([^"]*)"') or ""
    data.gold = tonumber(json:match('"gold":(%d+)')) or 0
    data.arcane_dust = tonumber(json:match('"arcane_dust":(%d+)')) or 0
    data.last_login = tonumber(json:match('"last_login":(%d+)')) or 0
    
    return data
end

--- 删除玩家缓存
function M.player.clear(player_id)
    local keys = {
        M.make_key("player", player_id, "basic"),
        M.make_key("player", player_id, "achievements"),
        M.make_key("player", player_id, "session"),
    }
    
    for _, key in ipairs(keys) do
        M.del(key)
    end
    return true
end

--- 保存成就进度
function M.player.save_achievement(player_id, achievement_id, progress)
    local key = M.make_key("player", player_id, "achievements")
    return M.hset(key, achievement_id, progress)
end

--- 加载所有成就进度
function M.player.load_achievements(player_id)
    local key = M.make_key("player", player_id, "achievements")
    local hash = M.hgetall(key)
    
    local achievements = {}
    for id, progress in pairs(hash) do
        achievements[id] = tonumber(progress) or 0
    end
    return achievements
end

--- 增加金币（原子操作）
function M.player.add_gold(player_id, amount)
    local key = M.make_key("player", player_id, "gold")
    return M.incrby(key, amount)
end

--- 设置玩家会话
function M.player.set_session(player_id, session_data, ttl)
    local key = M.make_key("session", player_id)
    
    local json = string.format(
        '{"player_id":"%s","login_time":%d,"last_active":%d}',
        player_id,
        session_data.login_time or os.time(),
        session_data.last_active or os.time()
    )
    
    return M.setex(key, json, ttl or 3600)
end

--- 获取玩家会话
function M.player.get_session(player_id)
    local key = M.make_key("session", player_id)
    local json = M.get(key)
    
    if not json then
        return nil
    end
    
    return {
        player_id = json:match('"player_id":"([^"]*)"'),
        login_time = tonumber(json:match('"login_time":(%d+)')) or 0,
        last_active = tonumber(json:match('"last_active":(%d+)')) or 0,
    }
end

--- 刷新会话活跃时间
function M.player.refresh_session(player_id, ttl)
    local key = M.make_key("session", player_id)
    return M.expire(key, ttl or 3600)
end

-- ==================== 测试 ====================

function M.test()
    skynet.error("[Redis] Running tests...")
    
    local info = M.info()
    skynet.error("[Redis] Mode:", info.using_redis and "Redis" or "Fallback")
    
    -- 基本 SET/GET
    local test_key = "test:redis:basic"
    if not M.set(test_key, "hello") then
        skynet.error("[Redis] Test FAILED: SET")
        return false
    end
    
    local value = M.get(test_key)
    if value ~= "hello" then
        skynet.error("[Redis] Test FAILED: GET")
        return false
    end
    
    -- EXISTS/DEL
    if not M.exists(test_key) then
        skynet.error("[Redis] Test FAILED: EXISTS")
        return false
    end
    
    M.del(test_key)
    if M.exists(test_key) then
        skynet.error("[Redis] Test FAILED: DEL")
        return false
    end
    
    -- Hash
    local hash_key = "test:redis:hash"
    M.hset(hash_key, "field1", "value1")
    M.hset(hash_key, "field2", "value2")
    
    if M.hget(hash_key, "field1") ~= "value1" then
        skynet.error("[Redis] Test FAILED: HGET")
        return false
    end
    
    local all = M.hgetall(hash_key)
    if all.field1 ~= "value1" or all.field2 ~= "value2" then
        skynet.error("[Redis] Test FAILED: HGETALL")
        return false
    end
    
    M.del(hash_key)
    
    skynet.error("[Redis] Test PASSED!")
    return true
end

return M
