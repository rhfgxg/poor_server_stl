-- Redis Bridge for Skynet
-- 通过 Socket 连接到 C++ RedisServer
-- C++ RedisServer 地址: 127.0.0.1:8003

local skynet = require "skynet"
local socket = require "skynet.socket"

local M = {}

-- 连接配置
local redis_server_fd = nil
local redis_config = nil
local using_cpp_server = false

-- 默认配置
local DEFAULT_CONFIG = {
    host = "127.0.0.1",
    port = 8003,  -- C++ RedisServer 端口
    timeout = 5000
}

-- ==================== 简单 JSON 编码/解码 ====================

local function json_encode(obj)
    if type(obj) ~= "table" then
        return string.format('"%s"', tostring(obj))
    end
    
    local parts = {}
    for k, v in pairs(obj) do
        local key = string.format('"%s"', tostring(k))
        local value
        if type(v) == "string" then
            value = string.format('"%s"', v)
        elseif type(v) == "table" then
            value = json_encode(v)
        else
            value = tostring(v)
        end
        table.insert(parts, key .. ":" .. value)
    end
    return "{" .. table.concat(parts, ",") .. "}"
end

local function json_decode(json_str)
    -- 简化的 JSON 解析（仅用于测试）
    -- 生产环境建议使用 lua-cjson
    if not json_str then
        return nil
    end
    
    -- 这里暂时返回原始字符串，由上层处理
    -- 真正的 JSON 解析需要更复杂的逻辑
    return json_str
end

-- ==================== 初始化 ====================

local function load_config()
    redis_config = DEFAULT_CONFIG
    skynet.error("[RedisBridge] Using default config")
    return true
end

-- 连接到 C++ RedisServer
function M.init()
    if redis_server_fd then
        skynet.error("[RedisBridge] Already connected")
        return true
    end
    
    load_config()
    
    -- 暂时禁用 C++ RedisServer 连接，直接使用内存模式
    -- 原因：C++ RedisServer 需要注册到中心服务器，涉及复杂的服务发现机制
    -- TODO: 等 C++ RedisServer 完善后再启用
    
    skynet.error("[RedisBridge] C++ RedisServer connection disabled (using fallback mode)")
    skynet.error("[RedisBridge] Reason: RedisServer needs registration to Central Server")
    return M.init_fallback()
end

-- 连接到 C++ RedisServer（暂时禁用）
function M.init_cpp_server()
    skynet.error("[RedisBridge] Trying to connect to C++ RedisServer...")
    
    local ok, fd = pcall(function()
        local conn_fd = socket.open(redis_config.host, redis_config.port)
        if not conn_fd then
            error("Failed to connect")
        end
        return conn_fd
    end)
    
    if not ok or not fd then
        skynet.error("[RedisBridge] Failed to connect to RedisServer:", tostring(fd))
        skynet.error("[RedisBridge] Falling back to memory mode")
        return M.init_fallback()
    end
    
    redis_server_fd = fd
    using_cpp_server = true
    skynet.error("[RedisBridge] ✓ Connected to C++ RedisServer at " .. redis_config.host .. ":" .. redis_config.port)
    return true
end

-- 降级方案：内存模式
function M.init_fallback()
    skynet.error("[RedisBridge] WARNING: Using in-memory fallback (NOT Redis!)")
    skynet.error("[RedisBridge] Data will NOT persist!")
    
    -- 使用内存存储
    redis_server_fd = {
        _storage = {},
        _expiry = {},
        _is_fallback = true
    }
    
    using_cpp_server = false
    return true
end

-- ==================== 底层通信 ====================

-- 发送请求到 C++ RedisServer
local function send_request(method, params)
    if not redis_server_fd then
        return nil, "not connected"
    end
    
    -- 降级模式：直接操作内存
    if redis_server_fd._is_fallback then
        return nil, "fallback mode - handled separately"
    end
    
    -- 构造请求（简单 JSON）
    local request_json = json_encode({
        method = method,
        params = params
    })
    
    -- 打包：[length(4字节)][json_body]
    local length = #request_json
    local header = string.pack(">I4", length)
    
    -- 发送
    local ok = pcall(function()
        socket.write(redis_server_fd, header .. request_json)
    end)
    
    if not ok then
        skynet.error("[RedisBridge] Failed to send request")
        return nil, "send failed"
    end
    
    -- 接收响应
    local response_header = socket.read(redis_server_fd, 4)
    if not response_header then
        return nil, "receive failed"
    end
    
    local response_length = string.unpack(">I4", response_header)
    local response_json = socket.read(redis_server_fd, response_length)
    
    if not response_json then
        return nil, "receive failed"
    end
    
    -- 简化：假设响应格式为 {"success":true,"result":"..."}
    -- 真实环境需要完整的 JSON 解析
    local result = string.match(response_json, '"result":"([^"]*)"')
    return result, nil
end

-- ==================== 基本命令（内存模式） ====================

local function memory_set(key, value)
    if redis_server_fd._is_fallback then
        redis_server_fd._storage[key] = value
        return "OK"
    end
    return nil
end

local function memory_get(key)
    if redis_server_fd._is_fallback then
        local value = redis_server_fd._storage[key]
        -- 检查过期
        if value and redis_server_fd._expiry[key] then
            if os.time() > redis_server_fd._expiry[key] then
                redis_server_fd._storage[key] = nil
                redis_server_fd._expiry[key] = nil
                return nil
            end
        end
        return value
    end
    return nil
end

local function memory_del(key)
    if redis_server_fd._is_fallback then
        redis_server_fd._storage[key] = nil
        redis_server_fd._expiry[key] = nil
        return 1
    end
    return 0
end

local function memory_exists(key)
    if redis_server_fd._is_fallback then
        return redis_server_fd._storage[key] ~= nil and 1 or 0
    end
    return 0
end

local function memory_setex(key, ttl, value)
    if redis_server_fd._is_fallback then
        redis_server_fd._storage[key] = value
        redis_server_fd._expiry[key] = os.time() + ttl
        return "OK"
    end
    return nil
end

local function memory_hset(key, field, value)
    if redis_server_fd._is_fallback then
        if type(redis_server_fd._storage[key]) ~= "table" then
            redis_server_fd._storage[key] = {}
        end
        redis_server_fd._storage[key][field] = value
        return 1
    end
    return 0
end

local function memory_hget(key, field)
    if redis_server_fd._is_fallback then
        if type(redis_server_fd._storage[key]) == "table" then
            return redis_server_fd._storage[key][field]
        end
    end
    return nil
end

local function memory_hgetall(key)
    if redis_server_fd._is_fallback then
        if type(redis_server_fd._storage[key]) == "table" then
            return redis_server_fd._storage[key]
        end
    end
    return {}
end

-- ==================== 基本命令（统一接口） ====================

function M.set(key, value)
    if not redis_server_fd then
        return false
    end
    
    if redis_server_fd._is_fallback then
        return memory_set(key, value) == "OK"
    end
    
    local result, err = send_request("set", {
        redis_type = "LOCAL",
        key = key,
        value = value
    })
    return result == "OK"
end

function M.setex(key, value, ttl_seconds)
    if not redis_server_fd then
        return false
    end
    
    if redis_server_fd._is_fallback then
        return memory_setex(key, ttl_seconds, value) == "OK"
    end
    
    local result, err = send_request("set", {
        redis_type = "LOCAL",
        key = key,
        value = value,
        ttl = ttl_seconds
    })
    return result == "OK"
end

function M.get(key)
    if not redis_server_fd then
        return nil
    end
    
    if redis_server_fd._is_fallback then
        return memory_get(key)
    end
    
    local result, err = send_request("get", {
        redis_type = "LOCAL",
        key = key
    })
    return result
end

function M.del(key)
    if not redis_server_fd then
        return false
    end
    
    if redis_server_fd._is_fallback then
        return memory_del(key) > 0
    end
    
    local result, err = send_request("del", {
        redis_type = "LOCAL",
        key = key
    })
    return result == "OK"
end

function M.exists(key)
    if not redis_server_fd then
        return false
    end
    
    if redis_server_fd._is_fallback then
        return memory_exists(key) > 0
    end
    
    local result, err = send_request("exists", {
        redis_type = "LOCAL",
        key = key
    })
    return result == "1"
end

function M.expire(key, ttl_seconds)
    if not redis_server_fd then
        return false
    end
    
    if redis_server_fd._is_fallback then
        if redis_server_fd._storage[key] then
            redis_server_fd._expiry[key] = os.time() + ttl_seconds
            return true
        end
        return false
    end
    
    local result, err = send_request("expire", {
        redis_type = "LOCAL",
        key = key,
        ttl = ttl_seconds
    })
    return result == "1"
end

function M.ttl(key)
    if not redis_server_fd then
        return -3
    end
    
    if redis_server_fd._is_fallback then
        if redis_server_fd._expiry[key] then
            local remaining = redis_server_fd._expiry[key] - os.time()
            return remaining > 0 and remaining or -2
        end
        return redis_server_fd._storage[key] and -1 or -2
    end
    
    local result, err = send_request("ttl", {
        redis_type = "LOCAL",
        key = key
    })
    return tonumber(result) or -3
end

-- ==================== Hash 命令 ====================

function M.hset(key, field, value)
    if not redis_server_fd then
        return false
    end
    
    if redis_server_fd._is_fallback then
        return memory_hset(key, field, value) > 0
    end
    
    local result, err = send_request("hset", {
        redis_type = "LOCAL",
        key = key,
        field = field,
        value = value
    })
    return result ~= nil
end

function M.hget(key, field)
    if not redis_server_fd then
        return nil
    end
    
    if redis_server_fd._is_fallback then
        return memory_hget(key, field)
    end
    
    local result, err = send_request("hget", {
        redis_type = "LOCAL",
        key = key,
        field = field
    })
    return result
end

function M.hdel(key, field)
    if not redis_server_fd then
        return false
    end
    
    if redis_server_fd._is_fallback then
        if type(redis_server_fd._storage[key]) == "table" then
            redis_server_fd._storage[key][field] = nil
            return true
        end
        return false
    end
    
    local result, err = send_request("hdel", {
        redis_type = "LOCAL",
        key = key,
        field = field
    })
    return result == "OK"
end

function M.hgetall(key)
    if not redis_server_fd then
        return {}
    end
    
    if redis_server_fd._is_fallback then
        return memory_hgetall(key)
    end
    
    local result, err = send_request("hgetall", {
        redis_type = "LOCAL",
        key = key
    })
    
    if type(result) == "table" then
        return result
    end
    return {}
end

-- ==================== 辅助函数 ====================

function M.make_key(namespace, ...)
    local parts = {namespace}
    for _, part in ipairs({...}) do
        table.insert(parts, tostring(part))
    end
    return table.concat(parts, ":")
end

-- ==================== 玩家数据接口 ====================

function M.save_player_basic(player_id, data)
    local key = M.make_key("player", player_id, "basic")
    local json_str = string.format(
        '{"level":%d,"exp":%d,"gold":%d,"last_login":%d}',
        data.level or 1,
        data.exp or 0,
        data.gold or 0,
        data.last_login or os.time()
    )
    return M.setex(key, json_str, 1800)
end

function M.load_player_basic(player_id)
    local key = M.make_key("player", player_id, "basic")
    local json_str = M.get(key)
    if not json_str then
        return nil
    end
    local data = {}
    data.level = tonumber(string.match(json_str, '"level":(%d+)')) or 1
    data.exp = tonumber(string.match(json_str, '"exp":(%d+)')) or 0
    data.gold = tonumber(string.match(json_str, '"gold":(%d+)')) or 0
    data.last_login = tonumber(string.match(json_str, '"last_login":(%d+)')) or os.time()
    return data
end

function M.save_player_achievement(player_id, achievement_id, progress)
    local key = M.make_key("player", player_id, "achievements")
    return M.hset(key, achievement_id, tostring(progress))
end

function M.load_player_achievements(player_id)
    local key = M.make_key("player", player_id, "achievements")
    local hash = M.hgetall(key)
    local achievements = {}
    for id, progress in pairs(hash) do
        achievements[id] = tonumber(progress) or 0
    end
    return achievements
end

function M.add_player_gold(player_id, amount)
    local key = M.make_key("player", player_id, "gold")
    
    if redis_server_fd._is_fallback then
        local current = tonumber(redis_server_fd._storage[key]) or 0
        local new_gold = current + amount
        redis_server_fd._storage[key] = tostring(new_gold)
        return new_gold
    end
    
    local result, err = send_request("incrby", {
        redis_type = "LOCAL",
        key = key,
        amount = amount
    })
    return tonumber(result)
end

-- ==================== 测试函数 ====================

function M.test()
    skynet.error("[RedisBridge] Running tests...")
    
    local test_key = "test:redis_bridge"
    local test_value = "Hello Redis!"
    
    if not M.set(test_key, test_value) then
        skynet.error("[RedisBridge] Test FAILED: SET")
        return false
    end
    
    local value = M.get(test_key)
    if value ~= test_value then
        skynet.error("[RedisBridge] Test FAILED: GET")
        skynet.error("  Expected:", test_value)
        skynet.error("  Got:", value)
        return false
    end
    
    if not M.exists(test_key) then
        skynet.error("[RedisBridge] Test FAILED: EXISTS")
        return false
    end
    
    if not M.del(test_key) then
        skynet.error("[RedisBridge] Test FAILED: DEL")
        return false
    end
    
    local hash_key = "test:hash"
    M.hset(hash_key, "field1", "value1")
    M.hset(hash_key, "field2", "value2")
    
    local field_value = M.hget(hash_key, "field1")
    if field_value ~= "value1" then
        skynet.error("[RedisBridge] Test FAILED: HGET")
        return false
    end
    
    local all_fields = M.hgetall(hash_key)
    if all_fields.field1 ~= "value1" or all_fields.field2 ~= "value2" then
        skynet.error("[RedisBridge] Test FAILED: HGETALL")
        return false
    end
    
    M.del(hash_key)
    
    skynet.error("[RedisBridge] Test PASSED!")
    return true
end

function M.get_info()
    local is_fallback = false
    local is_cpp_server = false
    
    if redis_server_fd then
        -- 检查是否为降级模式
        if type(redis_server_fd) == "table" and redis_server_fd._is_fallback then
            is_fallback = true
        else
            is_cpp_server = using_cpp_server
        end
    end
    
    return {
        initialized = redis_server_fd ~= nil,
        config = redis_config,
        using_fallback = is_fallback,
        using_cpp_server = is_cpp_server
    }
end

return M
