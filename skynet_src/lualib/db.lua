--[[
    数据库封装模块
    通过 RPC 调用 C++ DBServer 访问 MySQL
    
    使用方式：
        local db = require "db"
        
        -- 初始化（启动时调用一次）
        db.init()
        
        -- 玩家数据操作
        local player = db.player.load(player_id)
        db.player.save(player_id, player_data)
        db.player.create(player_id, user_id, nickname)
        
        -- 成就数据
        local achievements = db.achievement.load(player_id)
        db.achievement.save(player_id, achievements)
        
        -- 通用查询
        local rows = db.query("player_data", {player_id = "123"})
        db.insert("player_data", {player_id = "123", gold = 1000})
        db.update("player_data", {player_id = "123"}, {gold = 2000})
]]

local skynet = require "skynet"
local socket = require "skynet.socket"

local M = {}
M.player = {}      -- 玩家数据接口
M.achievement = {} -- 成就数据接口

-- ==================== 配置 ====================

local config = {
    host = "127.0.0.1",
    port = 50053,           -- C++ DBServer Socket 端口
    timeout = 5000,
    database = "poor_hearthstone",
    retry_count = 3,
    retry_delay = 1000,     -- 毫秒
}

-- ==================== 连接管理 ====================

local using_mock = false
local mock_storage = nil

-- ==================== JSON 工具 ====================

local function json_encode_value(v)
    if type(v) == "string" then
        -- 转义特殊字符
        local escaped = v:gsub('\\', '\\\\'):gsub('"', '\\"'):gsub('\n', '\\n')
        return '"' .. escaped .. '"'
    elseif type(v) == "number" then
        return tostring(v)
    elseif type(v) == "boolean" then
        return v and "true" or "false"
    elseif type(v) == "table" then
        local parts = {}
        -- 检查是否为数组
        if #v > 0 then
            for _, item in ipairs(v) do
                table.insert(parts, json_encode_value(item))
            end
            return "[" .. table.concat(parts, ",") .. "]"
        else
            for k, val in pairs(v) do
                table.insert(parts, '"' .. tostring(k) .. '":' .. json_encode_value(val))
            end
            return "{" .. table.concat(parts, ",") .. "}"
        end
    else
        return "null"
    end
end

local function json_encode(obj)
    if type(obj) ~= "table" then
        return json_encode_value(obj)
    end
    
    local parts = {}
    for k, v in pairs(obj) do
        table.insert(parts, '"' .. tostring(k) .. '":' .. json_encode_value(v))
    end
    return "{" .. table.concat(parts, ",") .. "}"
end

local function json_decode_simple(json_str)
    if not json_str then
        return nil
    end
    
    local result = {}
    
    -- 提取 success
    local success = json_str:match('"success":(%w+)')
    result.success = success == "true"
    
    -- 提取 message
    result.message = json_str:match('"message":"([^"]*)"') or ""
    
    -- 保留原始字符串用于进一步解析
    result.raw = json_str
    
    return result
end

-- ==================== 底层通信 ====================

local function send_request(method, table_name, data, query)
    -- Mock 模式直接返回
    if using_mock then
        return nil, "mock mode"
    end
    
    -- 构造请求
    local request = {
        method = method,
        database = config.database,
    }
    if table_name then
        request["table"] = table_name
    end
    if data then
        request.data = data
    end
    if query then
        request.query = query
    end
    
    local request_json = json_encode(request)
    
    -- 每次请求新建连接（短连接模式，避免连接断开问题）
    local socket_fd = socket.open(config.host, config.port)
    if not socket_fd then
        skynet.error("[DB] Failed to connect to DBServer")
        return nil, "connection failed"
    end
    
    -- 打包：[4字节长度][JSON]
    local header = string.pack(">I4", #request_json)
    
    local ok, err = pcall(function()
        socket.write(socket_fd, header .. request_json)
    end)
    
    if not ok then
        socket.close(socket_fd)
        skynet.error("[DB] Send failed:", err)
        return nil, "send failed"
    end
    
    -- 接收响应
    local response_header = socket.read(socket_fd, 4)
    if not response_header or #response_header ~= 4 then
        socket.close(socket_fd)
        return nil, "receive header failed"
    end
    
    local response_length = string.unpack(">I4", response_header)
    if response_length == 0 or response_length > 1024 * 1024 then
        socket.close(socket_fd)
        return nil, "invalid response length"
    end
    
    local response_json = socket.read(socket_fd, response_length)
    socket.close(socket_fd)
    
    if not response_json or #response_json ~= response_length then
        return nil, "receive body failed"
    end
    
    return json_decode_simple(response_json), nil
end

-- ==================== Mock 数据库 ====================

local function mock_init()
    skynet.error("[DB] Using in-memory Mock database")
    skynet.error("[DB] WARNING: Data will NOT persist to MySQL!")
    
    mock_storage = {
        player_data = {},
        player_achievements_quests = {},
        player_collection = {},
    }
    using_mock = true
    return true
end

local function mock_read(table_name, query)
    if not mock_storage[table_name] then
        return nil
    end
    
    local player_id = query.player_id
    if player_id then
        return mock_storage[table_name][tostring(player_id)]
    end
    return nil
end

local function mock_write(table_name, data)
    if not mock_storage[table_name] then
        mock_storage[table_name] = {}
    end
    
    local player_id = data.player_id
    if player_id then
        mock_storage[table_name][tostring(player_id)] = data
        return true
    end
    return false
end

local function mock_update(table_name, query, data)
    local player_id = query.player_id
    if not player_id then
        return false
    end
    
    local existing = mock_storage[table_name] and mock_storage[table_name][tostring(player_id)]
    if not existing then
        return false
    end
    
    for k, v in pairs(data) do
        existing[k] = v
    end
    return true
end

-- ==================== 初始化接口 ====================

--- 初始化数据库连接
--- @param cfg table|nil 配置（可选）
--- @return boolean 是否成功
function M.init(cfg)
    if using_mock or mock_storage then
        skynet.error("[DB] Already initialized")
        return true
    end
    
    -- 合并配置
    if cfg then
        for k, v in pairs(cfg) do
            config[k] = v
        end
    end
    
    skynet.error("[DB] Initializing...")
    skynet.error("[DB] Target: " .. config.host .. ":" .. config.port)
    
    -- 尝试连接 C++ DBServer
    local test_fd = socket.open(config.host, config.port)
    if test_fd then
        socket.close(test_fd)
        skynet.error("[DB] DBServer is reachable")
        using_mock = false
        return true
    else
        skynet.error("[DB] DBServer not reachable, using Mock mode")
        return mock_init()
    end
end

--- 获取连接状态信息
function M.info()
    return {
        connected = not using_mock,
        using_mock = using_mock,
        config = {
            host = config.host,
            port = config.port,
            database = config.database,
        }
    }
end

-- ==================== 通用 CRUD 接口 ====================

--- 查询数据
--- @param table_name string 表名
--- @param query table 查询条件
--- @return table|nil 查询结果
function M.query(table_name, query)
    if using_mock then
        return mock_read(table_name, query)
    end
    
    local response, err = send_request("Read", table_name, nil, query)
    if not response or not response.success then
        skynet.error("[DB] Query failed:", err or (response and response.message))
        return nil
    end
    
    -- TODO: 解析 response.raw 中的 results
    return response
end

--- 插入数据
--- @param table_name string 表名
--- @param data table 数据
--- @return boolean 是否成功
function M.insert(table_name, data)
    if using_mock then
        return mock_write(table_name, data)
    end
    
    local response, err = send_request("Create", table_name, data, nil)
    if not response or not response.success then
        skynet.error("[DB] Insert failed:", err or (response and response.message))
        return false
    end
    
    return true
end

--- 更新数据
--- @param table_name string 表名
--- @param query table 查询条件
--- @param data table 更新数据
--- @return boolean 是否成功
function M.update(table_name, query, data)
    if using_mock then
        return mock_update(table_name, query, data)
    end
    
    -- 合并 query 和 data
    local merged = {}
    for k, v in pairs(query) do
        merged[k] = v
    end
    for k, v in pairs(data) do
        merged[k] = v
    end
    
    local response, err = send_request("Update", table_name, merged, query)
    if not response or not response.success then
        skynet.error("[DB] Update failed:", err or (response and response.message))
        return false
    end
    
    return true
end

--- 删除数据
--- @param table_name string 表名
--- @param query table 查询条件
--- @return boolean 是否成功
function M.delete(table_name, query)
    if using_mock then
        local player_id = query.player_id
        if player_id and mock_storage[table_name] then
            mock_storage[table_name][tostring(player_id)] = nil
            return true
        end
        return false
    end
    
    local response, err = send_request("Delete", table_name, nil, query)
    if not response or not response.success then
        skynet.error("[DB] Delete failed:", err or (response and response.message))
        return false
    end
    
    return true
end

-- ==================== 玩家数据接口 ====================

--- 加载玩家数据
--- @param player_id number|string 玩家ID
--- @return table|nil 玩家数据
function M.player.load(player_id)
    player_id = tostring(player_id)
    
    if using_mock then
        local row = mock_read("player_data", {player_id = player_id})
        if not row then
            return nil
        end
        return {
            player_id = row.player_id,
            user_id = row.user_id or player_id,
            nickname = row.nickname or ("Player_" .. player_id),
            gold = row.gold or 0,
            arcane_dust = row.arcane_dust or 0,
            last_login = row.last_login or os.time(),
        }
    end
    
    local response, err = send_request("Read", "player_data", nil, {player_id = player_id})
    if not response or not response.success then
        return nil
    end
    
    -- 从 raw 中解析字段
    local raw = response.raw or ""
    return {
        player_id = tonumber(raw:match('"player_id":"?(%d+)"?')) or player_id,
        user_id = tonumber(raw:match('"user_id":"?(%d+)"?')) or player_id,
        nickname = raw:match('"nickname":"([^"]*)"') or ("Player_" .. player_id),
        gold = tonumber(raw:match('"gold":"?(%d+)"?')) or 0,
        arcane_dust = tonumber(raw:match('"arcane_dust":"?(%d+)"?')) or 0,
        last_login = raw:match('"last_login":"([^"]*)"') or os.time(),
    }
end

--- 保存玩家数据
--- @param player_id number|string 玩家ID
--- @param data table 玩家数据
--- @return boolean 是否成功
function M.player.save(player_id, data)
    player_id = tostring(player_id)
    
    local db_data = {
        player_id = player_id,
        user_id = data.user_id or player_id,
        nickname = data.nickname or ("Player_" .. player_id),
        gold = data.gold or 0,
        arcane_dust = data.arcane_dust or 0,
        last_login = os.date("%Y-%m-%d %H:%M:%S", data.last_login or os.time()),
    }
    
    if using_mock then
        local existing = mock_read("player_data", {player_id = player_id})
        if existing then
            return mock_update("player_data", {player_id = player_id}, db_data)
        else
            return mock_write("player_data", db_data)
        end
    end
    
    local response, err = send_request("Create", "player_data", db_data, nil)
    return response and response.success
end

--- 创建新玩家
--- @param player_id number|string 玩家ID
--- @param user_id number|string 用户ID
--- @param nickname string 昵称
--- @return boolean 是否成功
function M.player.create(player_id, user_id, nickname)
    return M.player.save(player_id, {
        user_id = user_id or player_id,
        nickname = nickname or ("Player_" .. player_id),
        gold = 1000,        -- 初始金币
        arcane_dust = 0,
        last_login = os.time(),
    })
end

--- 检查玩家是否存在
--- @param player_id number|string 玩家ID
--- @return boolean 是否存在
function M.player.exists(player_id)
    local data = M.player.load(player_id)
    return data ~= nil
end

--- 更新玩家金币
--- @param player_id number|string 玩家ID
--- @param gold number 金币数量
--- @return boolean 是否成功
function M.player.update_gold(player_id, gold)
    if using_mock then
        return mock_update("player_data", {player_id = tostring(player_id)}, {gold = gold})
    end
    
    local response, err = send_request("Update", "player_data", 
        {player_id = tostring(player_id), gold = gold}, 
        {player_id = tostring(player_id)})
    return response and response.success
end

-- ==================== 成就数据接口 ====================

--- 加载玩家成就数据
--- @param player_id number|string 玩家ID
--- @return table 成就数据 {progress = {}, completed = {}}
function M.achievement.load(player_id)
    player_id = tostring(player_id)
    
    if using_mock then
        local row = mock_read("player_achievements_quests", {player_id = player_id})
        if row and row.achievements then
            return row.achievements
        end
        return {progress = {}, completed = {}}
    end
    
    local response, err = send_request("Read", "player_achievements_quests", nil, {player_id = player_id})
    if not response or not response.success then
        return {progress = {}, completed = {}}
    end
    
    -- TODO: 解析 JSON 格式的成就数据
    return {progress = {}, completed = {}}
end

--- 保存玩家成就数据
--- @param player_id number|string 玩家ID
--- @param achievements table 成就数据 {progress = {}, completed = {}}
--- @return boolean 是否成功
function M.achievement.save(player_id, achievements)
    player_id = tostring(player_id)
    
    -- 序列化成就数据
    local progress_parts = {}
    for id, value in pairs(achievements.progress or {}) do
        table.insert(progress_parts, '"' .. id .. '":' .. tostring(value))
    end
    
    local completed_parts = {}
    for id, _ in pairs(achievements.completed or {}) do
        table.insert(completed_parts, '"' .. id .. '":true')
    end
    
    local achievements_json = string.format(
        '{"progress":{%s},"completed":{%s}}',
        table.concat(progress_parts, ","),
        table.concat(completed_parts, ",")
    )
    
    if using_mock then
        local data = {
            player_id = player_id,
            achievements = achievements,
        }
        local existing = mock_read("player_achievements_quests", {player_id = player_id})
        if existing then
            return mock_update("player_achievements_quests", {player_id = player_id}, data)
        else
            return mock_write("player_achievements_quests", data)
        end
    end
    
    local response, err = send_request("Create", "player_achievements_quests", 
        {player_id = player_id, achievements = achievements_json}, nil)
    return response and response.success
end

--- 更新单个成就进度
--- @param player_id number|string 玩家ID
--- @param achievement_id string 成就ID
--- @param progress number 进度值
--- @return boolean 是否成功
function M.achievement.update_progress(player_id, achievement_id, progress)
    local achievements = M.achievement.load(player_id)
    achievements.progress[achievement_id] = progress
    return M.achievement.save(player_id, achievements)
end

--- 标记成就完成
--- @param player_id number|string 玩家ID
--- @param achievement_id string 成就ID
--- @return boolean 是否成功
function M.achievement.complete(player_id, achievement_id)
    local achievements = M.achievement.load(player_id)
    achievements.completed[achievement_id] = true
    return M.achievement.save(player_id, achievements)
end

-- ==================== 完整数据操作 ====================

--- 加载玩家完整数据（包含成就）
--- @param player_id number|string 玩家ID
--- @return table|nil 完整玩家数据
function M.player.load_full(player_id)
    local basic = M.player.load(player_id)
    if not basic then
        return nil
    end
    
    local achievements = M.achievement.load(player_id)
    
    return {
        player_id = basic.player_id,
        user_id = basic.user_id,
        nickname = basic.nickname,
        gold = basic.gold,
        arcane_dust = basic.arcane_dust,
        achievements = achievements,
        cards = {},
        decks = {},
        last_login = basic.last_login,
    }
end

--- 保存玩家完整数据（包含成就）
--- @param player_id number|string 玩家ID
--- @param data table 完整玩家数据
--- @return boolean 是否成功
function M.player.save_full(player_id, data)
    -- 保存基础数据
    if not M.player.save(player_id, data) then
        return false
    end
    
    -- 保存成就数据
    if data.achievements then
        M.achievement.save(player_id, data.achievements)
    end
    
    return true
end

-- ==================== 测试 ====================

function M.test()
    skynet.error("[DB] Running tests...")
    
    local info = M.info()
    skynet.error("[DB] Mode:", info.using_mock and "Mock" or "DBServer")
    
    -- 使用时间戳作为测试 player_id
    local test_player_id = "test_" .. os.time()
    
    -- 测试创建玩家
    if not M.player.create(test_player_id, 1, "TestPlayer") then
        skynet.error("[DB] Test FAILED: create player")
        return false
    end
    skynet.error("[DB] Player created:", test_player_id)
    
    -- 测试加载玩家
    local player = M.player.load(test_player_id)
    if not player then
        skynet.error("[DB] Test FAILED: load player")
        return false
    end
    skynet.error("[DB] Player loaded:", player.nickname, "gold:", player.gold)
    
    -- 测试更新
    player.gold = 5000
    if not M.player.save(test_player_id, player) then
        skynet.error("[DB] Test FAILED: save player")
        return false
    end
    
    -- 验证更新
    local updated = M.player.load(test_player_id)
    if not updated or updated.gold ~= 5000 then
        skynet.error("[DB] Test FAILED: verify update")
        return false
    end
    
    -- 测试成就
    M.achievement.update_progress(test_player_id, "first_win", 1)
    M.achievement.complete(test_player_id, "tutorial")
    
    local achievements = M.achievement.load(test_player_id)
    skynet.error("[DB] Achievements loaded, progress count:", 
        achievements.progress and #achievements.progress or 0)
    
    skynet.error("[DB] Test PASSED!")
    return true
end

return M
