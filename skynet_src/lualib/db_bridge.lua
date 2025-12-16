-- DB Bridge for Skynet
-- 通过 Socket 连接到 C++ DBServer (gRPC on Port 50052)
-- 统一数据访问接口

local skynet = require "skynet"
local socket = require "skynet.socket"

local M = {}

-- 连接配置
local db_server_fd = nil
local db_config = nil
local using_cpp_server = false

-- 默认配置
local DEFAULT_CONFIG = {
    host = "127.0.0.1",
    port = 50053,  -- ✅ C++ DBServer Socket 端口（修改为 50053）
    timeout = 5000,
    database = "poor_hearthstone"
}

-- ==================== 简单 Protobuf 编码/解码 ====================
-- 注意：这是简化版本，生产环境建议使用 lua-protobuf

local function encode_request(method, database, table_name, data, query)
    -- 简化的请求格式（JSON）
    local request = {
        method = method,
        database = database or DEFAULT_CONFIG.database,
        table = table_name,
        data = data,
        query = query
    }
    
    -- 转换为 JSON 字符串
    local json_str = "{"
    json_str = json_str .. string.format('"method":"%s"', request.method)
    json_str = json_str .. string.format(',"database":"%s"', request.database)
    if request.table then
        json_str = json_str .. string.format(',"table":"%s"', request.table)
    end
    
    if request.data then
        json_str = json_str .. ',"data":{'
        local first = true
        for k, v in pairs(request.data) do
            if not first then json_str = json_str .. "," end
            json_str = json_str .. string.format('"%s":"%s"', k, tostring(v))
            first = false
        end
        json_str = json_str .. "}"
    end
    
    if request.query then
        json_str = json_str .. ',"query":{'
        local first = true
        for k, v in pairs(request.query) do
            if not first then json_str = json_str .. "," end
            json_str = json_str .. string.format('"%s":"%s"', k, tostring(v))
            first = false
        end
        json_str = json_str .. "}"
    end
    
    json_str = json_str .. "}"
    return json_str
end

local function decode_response(json_str)
    -- 简化的响应解析
    if not json_str then
        return {success = false, message = "empty response"}
    end
    
    local success = string.match(json_str, '"success":(%w+)')
    local message = string.match(json_str, '"message":"([^"]*)"')
    
    return {
        success = success == "true",
        message = message or "",
        raw = json_str
    }
end

-- ==================== 初始化 ====================

local function load_config()
    db_config = DEFAULT_CONFIG
    skynet.error("[DBBridge] Using default config")
    return true
end

-- 连接到 C++ DBServer
function M.init()
    if db_server_fd then
        skynet.error("[DBBridge] Already connected")
        return true
    end
    
    load_config()
    
    -- ✅ 启用 C++ DBServer Socket 连接
    skynet.error("[DBBridge] Connecting to C++ DBServer Socket...")
    skynet.error("[DBBridge] Target: " .. db_config.host .. ":" .. db_config.port)
    
    local ok = M.init_cpp_server()
    if ok then
        skynet.error("[DBBridge] ✓ Successfully connected to C++ DBServer")
        return true
    else
        skynet.error("[DBBridge] Connection failed, falling back to Mock mode")
        return M.init_mock()
    end
end

-- 连接到 C++ DBServer（未来实现）
function M.init_cpp_server()
    skynet.error("[DBBridge] Connecting to C++ DBServer...")
    
    local ok, fd = pcall(function()
        return socket.open(db_config.host, db_config.port)
    end)
    
    if not ok or not fd then
        skynet.error("[DBBridge] Failed to connect to DBServer:", tostring(fd))
        skynet.error("[DBBridge] Falling back to Mock mode")
        return M.init_mock()
    end
    
    db_server_fd = fd
    using_cpp_server = true
    skynet.error("[DBBridge] ✓ Connected to C++ DBServer at " .. db_config.host .. ":" .. db_config.port)
    return true
end

-- Mock 数据库（降级方案）
function M.init_mock()
    skynet.error("[DBBridge] WARNING: Using in-memory Mock database!")
    skynet.error("[DBBridge] Data will NOT persist to real MySQL")
    
    db_server_fd = {
        _tables = {
            player_data = {},
            player_achievements_quests = {},
            player_collection = {}
        },
        _is_mock = true
    }
    
    using_cpp_server = false
    return true
end

-- ==================== 底层通信 ====================

local function send_request(method, database, table_name, data, query)
    if not db_server_fd then
        return nil, "not connected"
    end
    
    -- Mock 模式：直接处理
    if type(db_server_fd) == "table" and db_server_fd._is_mock then
        return nil, "mock mode - handled separately"
    end
    
    -- ✅ 真实连接：每次请求重新连接（避免连接断开问题）
    local socket_fd = socket.open(db_config.host, db_config.port)
    if not socket_fd then
        skynet.error("[DBBridge] Failed to reconnect for request")
        return nil, "connection failed"
    end
    
    -- 发送到 C++ DBServer
    local request_json = encode_request(method, database, table_name, data, query)
    
    skynet.error("[DBBridge] Sending request:", request_json:sub(1, 100))
    
    -- 打包：[length(4字节)][json_body]
    local length = #request_json
    local header = string.pack(">I4", length)
    
    -- ✅ 修复：合并 header 和 body 一次发送
    local message = header .. request_json
    
    local ok, err = pcall(function()
        socket.write(socket_fd, message)
    end)
    
    if not ok then
        skynet.error("[DBBridge] Failed to send request:", err)
        socket.close(socket_fd)
        return nil, "send failed"
    end
    
    skynet.error("[DBBridge] Request sent, waiting for response...")
    
    -- 接收响应
    local response_header = socket.read(socket_fd, 4)
    if not response_header or #response_header ~= 4 then
        skynet.error("[DBBridge] Failed to receive response header")
        socket.close(socket_fd)
        return nil, "receive header failed"
    end
    
    local response_length = string.unpack(">I4", response_header)
    skynet.error("[DBBridge] Response length:", response_length)
    
    if response_length == 0 or response_length > 1024 * 1024 then
        skynet.error("[DBBridge] Invalid response length:", response_length)
        socket.close(socket_fd)
        return nil, "invalid response length"
    end
    
    local response_json = socket.read(socket_fd, response_length)
    
    if not response_json or #response_json ~= response_length then
        skynet.error("[DBBridge] Failed to receive response body")
        socket.close(socket_fd)
        return nil, "receive body failed"
    end
    
    skynet.error("[DBBridge] Response received:", response_json:sub(1, 200))
    
    -- ✅ 关闭连接
    socket.close(socket_fd)
    
    return decode_response(response_json), nil
end

-- ==================== Mock 数据库操作 ====================

local function mock_create(table_name, data)
    -- ✅ 修复：这个函数只在 Mock 模式下调用，所以需要正向检查
    if type(db_server_fd) ~= "table" or not db_server_fd._is_mock then
        return false
    end
    
    if not db_server_fd._tables[table_name] then
        db_server_fd._tables[table_name] = {}
    end
    
    -- 使用 player_id 作为 key
    local player_id = data.player_id
    if player_id then
        db_server_fd._tables[table_name][player_id] = data
        return true
    end
    
    return false
end

local function mock_read(table_name, query)
    -- ✅ 修复：这个函数只在 Mock 模式下调用
    if type(db_server_fd) ~= "table" or not db_server_fd._is_mock then
        return nil
    end
    
    if not db_server_fd._tables[table_name] then
        return nil
    end
    
    -- 简单查询：通过 player_id
    local player_id = query.player_id
    if player_id then
        return db_server_fd._tables[table_name][player_id]
    end
    
    return nil
end

local function mock_update(table_name, query, data)
    -- ✅ 修复：这个函数只在 Mock 模式下调用
    if type(db_server_fd) ~= "table" or not db_server_fd._is_mock then
        return false
    end
    
    local player_id = query.player_id
    if not player_id or not db_server_fd._tables[table_name][player_id] then
        return false
    end
    
    -- 更新数据
    for k, v in pairs(data) do
        db_server_fd._tables[table_name][player_id][k] = v
    end
    
    return true
end

-- ==================== 玩家数据 CRUD ====================

function M.load_player_basic(player_id)
    if not db_server_fd then
        return nil
    end
    
    -- Mock 模式
    if type(db_server_fd) == "table" and db_server_fd._is_mock then
        local row = mock_read("player_data", {player_id = player_id})
        if not row then
            skynet.error("[DBBridge] Player not found:", player_id)
            return nil
        end
        
        return {
            player_id = row.player_id,
            user_id = row.user_id or player_id,
            nickname = row.nickname or ("Player_" .. player_id),
            gold = row.gold or 0,
            arcane_dust = row.arcane_dust or 0,
            created_at = row.created_at or os.time(),
            updated_at = row.updated_at or os.time(),
            last_login = row.last_login or os.time()
        }
    end
    
    -- C++ DBServer 模式
    local response, err = send_request("Read", nil, "player_data", nil, {player_id = tostring(player_id)})
    if not response or not response.success then
        skynet.error("[DBBridge] Failed to load player:", err or response.message)
        return nil
    end
    
    -- ✅ 解析 response.results
    if not response.raw then
        skynet.error("[DBBridge] No raw response data")
        return nil
    end
    
    -- 简单的 JSON 解析提取 results
    local results_start = response.raw:find('"results":%[')
    if not results_start then
        skynet.error("[DBBridge] No results in response")
        return nil
    end
    
    -- 提取字段值（简化版本）
    local function extract_field(field_name)
        local pattern = '"' .. field_name .. '":"([^"]*)"'
        local value = response.raw:match(pattern)
        return value
    end
    
    local player_data = {
        player_id = tonumber(extract_field("player_id")) or player_id,
        user_id = tonumber(extract_field("user_id")) or player_id,
        nickname = extract_field("nickname") or ("Player_" .. player_id),
        gold = tonumber(extract_field("gold")) or 0,
        arcane_dust = tonumber(extract_field("arcane_dust")) or 0,
        last_login = extract_field("last_login") or os.time()
    }
    
    skynet.error("[DBBridge] Player loaded:", player_id)
    return player_data
end

function M.save_player_basic(player_id, data)
    if not db_server_fd then
        return false
    end
    
    -- 准备数据（移除 level 和 exp）
    local db_data = {
        player_id = player_id,
        user_id = data.user_id or player_id,
        nickname = data.nickname or ("Player_" .. player_id),
        gold = data.gold or 0,
        arcane_dust = data.arcane_dust or 0,  -- ✅ 保留奥术之尘
        -- ✅ 转换时间戳为 MySQL TIMESTAMP 格式
        last_login = os.date("%Y-%m-%d %H:%M:%S", data.last_login or os.time())
    }
    
    -- ✅ 修复：Mock 模式检查，添加 type() 判断
    if type(db_server_fd) == "table" and db_server_fd._is_mock then
        -- 尝试读取
        local existing = mock_read("player_data", {player_id = player_id})
        
        if existing then
            -- 更新
            return mock_update("player_data", {player_id = player_id}, db_data)
        else
            -- 创建
            return mock_create("player_data", db_data)
        end
    end
    
    -- C++ DBServer 模式：使用 REPLACE（Create 或 Update）
    local response, err = send_request("Create", nil, "player_data", db_data, nil)
    if not response or not response.success then
        skynet.error("[DBBridge] Failed to save player:", err or response.message)
        return false
    end
    
    skynet.error("[DBBridge] Player saved:", player_id)
    return true
end

function M.create_player(player_id, user_id, nickname)
    return M.save_player_basic(player_id, {
        user_id = user_id or player_id,
        nickname = nickname or ("Player_" .. player_id),
        gold = 1000,
        arcane_dust = 0,  -- ✅ 保留奥术之尘
        last_login = os.time()
    })
end

-- ==================== 成就数据 CRUD（JSON 格式）====================

function M.load_player_achievements(player_id)
    if not db_server_fd then
        return {progress = {}, completed = {}}
    end
    
    -- Mock 模式
    if type(db_server_fd) == "table" and db_server_fd._is_mock then
        local row = mock_read("player_achievements_quests", {player_id = player_id})
        if not row or not row.achievements then
            return {progress = {}, completed = {}}
        end
        
        -- 假设已经是 Lua table
        if type(row.achievements) == "table" then
            return row.achievements
        end
    end
    
    return {progress = {}, completed = {}}
end

function M.save_player_achievements(player_id, achievements)
    if not db_server_fd or not achievements then
        return false
    end
    
    -- 手动构建 JSON
    local progress_json = "{"
    local first = true
    for ach_id, progress in pairs(achievements.progress or {}) do
        if not first then progress_json = progress_json .. "," end
        progress_json = progress_json .. string.format('"%s":%d', ach_id, progress)
        first = false
    end
    progress_json = progress_json .. "}"
    
    local completed_json = "{"
    first = true
    for ach_id, _ in pairs(achievements.completed or {}) do
        if not first then completed_json = completed_json .. "," end
        completed_json = completed_json .. string.format('"%s":true', ach_id)
        first = false
    end
    completed_json = completed_json .. "}"
    
    local achievements_json = string.format('{"progress":%s,"completed":%s}', 
        progress_json, completed_json)
    
    -- Mock 模式
    if type(db_server_fd) == "table" and db_server_fd._is_mock then
        local data = {
            player_id = player_id,
            achievements = achievements  -- 直接存储 table
        }
        
        local existing = mock_read("player_achievements_quests", {player_id = player_id})
        if existing then
            return mock_update("player_achievements_quests", {player_id = player_id}, data)
        else
            return mock_create("player_achievements_quests", data)
        end
    end
    
    -- C++ DBServer 模式
    local response, err = send_request("Create", nil, "player_achievements_quests", 
        {player_id = tostring(player_id), achievements = achievements_json}, nil)
    
    if not response or not response.success then
        skynet.error("[DBBridge] Failed to save achievements:", err or response.message)
        return false
    end
    
    skynet.error("[DBBridge] Achievements saved:", player_id)
    return true
end

-- ==================== 完整数据操作 ====================

function M.save_player_full(player_id, player_data)
    if not db_server_fd then
        return false
    end
    
    -- 1. 保存基础数据（移除 level 和 exp）
    if not M.save_player_basic(player_id, {
        user_id = player_data.user_id,
        nickname = player_data.nickname,
        gold = player_data.gold,
        arcane_dust = player_data.arcane_dust,  -- ✅ 保留奥术之尘
        last_login = player_data.last_login
    }) then
        return false
    end
    
    -- 2. 保存成就数据
    if player_data.achievements then
        M.save_player_achievements(player_id, player_data.achievements)
    end
    
    skynet.error("[DBBridge] Full player data saved:", player_id)
    return true
end

function M.load_player_full(player_id)
    if not db_server_fd then
        return nil
    end
    
    -- 1. 加载基础数据
    local basic_data = M.load_player_basic(player_id)
    if not basic_data then
        skynet.error("[DBBridge] Player not found:", player_id)
        return nil
    end
    
    -- 2. 加载成就数据
    local achievements = M.load_player_achievements(player_id)
    
    -- 3. 组装完整数据（移除 level 和 exp）
    return {
        player_id = basic_data.player_id,
        user_id = basic_data.user_id,
        nickname = basic_data.nickname,
        gold = basic_data.gold,
        arcane_dust = basic_data.arcane_dust,  -- ✅ 保留奥术之尘
        achievements = achievements,
        cards = {},
        decks = {},
        created_at = basic_data.created_at,
        updated_at = basic_data.updated_at,
        last_login = basic_data.last_login
    }
end

-- ==================== 测试函数 ====================

function M.test()
    skynet.error("[DBBridge] Running tests...")
    
    -- ✅ 使用当前时间戳作为 player_id，避免重复
    local test_player_id = os.time() % 100000  -- 使用时间戳的后5位
    skynet.error("[DBBridge] Test player_id:", test_player_id)
    
    -- 测试保存（移除 level 和 exp）
    local save_ok = M.save_player_basic(test_player_id, {
        user_id = 1,
        nickname = "TestPlayer",
        gold = 20000,
        arcane_dust = 500,  -- ✅ 使用奥术之尘代替
        last_login = os.time()
    })
    
    if not save_ok then
        skynet.error("[DBBridge] Test FAILED: save_player_basic")
        return false
    end
    
    skynet.error("[DBBridge] Test data saved successfully")
    
    -- 测试加载
    local loaded_data = M.load_player_basic(test_player_id)
    if not loaded_data then
        skynet.error("[DBBridge] Test FAILED: load_player_basic")
        return false
    end
    
    skynet.error("[DBBridge] Test data loaded successfully")
    skynet.error("[DBBridge]   player_id:", loaded_data.player_id)
    skynet.error("[DBBridge]   nickname:", loaded_data.nickname)
    skynet.error("[DBBridge]   gold:", loaded_data.gold)
    skynet.error("[DBBridge]   arcane_dust:", loaded_data.arcane_dust)
    
    -- ✅ 修改验证逻辑：检查 gold 和 arcane_dust
    if loaded_data.gold ~= 20000 or loaded_data.arcane_dust ~= 500 then
        skynet.error("[DBBridge] Test FAILED: data mismatch")
        skynet.error("  Expected: gold=20000, arcane_dust=500")
        skynet.error("  Got: gold=" .. loaded_data.gold .. ", arcane_dust=" .. loaded_data.arcane_dust)
        return false
    end
    
    skynet.error("[DBBridge] Test PASSED!")
    return true
end

function M.get_info()
    local is_mock = false
    local is_cpp_server = false
    
    if db_server_fd then
        if type(db_server_fd) == "table" and db_server_fd._is_mock then
            is_mock = true
        else
            is_cpp_server = using_cpp_server
        end
    end
    
    return {
        connected = db_server_fd ~= nil,
        config = db_config,
        using_mock = is_mock,
        using_cpp_server = is_cpp_server,
        database = db_config and db_config.database or "unknown"
    }
end

return M
