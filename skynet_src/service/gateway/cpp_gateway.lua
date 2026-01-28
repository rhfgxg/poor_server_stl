--[[
    C++ Gateway Service (统一双向网关)
    
    职责�?    1. 接收来自 C++ Gateway �?TCP 连接（被动模式，C++ �?Skynet�?    2. 提供接口�?Skynet 服务�?C++ 发送请求（主动模式，Skynet �?C++�?    3. 双向复用同一�?TCP 连接
    
    协议格式：[4字节长度][2字节类型][Protobuf数据]
    
    架构�?    ┌─────────────────────────────────────────────────────────�?    �?                   Skynet 集群                          �?    �? player_agent / battle / matching / ...                 �?    �?        �?                                              �?    �?        �?skynet.call(gateway, "lua", "request", ...)   �?    �?        �?                                              �?    �? ┌──────────────────────────────────────�?              �?    �? �?        cpp_gateway (本服�?          �?              �?    �? �? - 监听端口接收 C++ 请求              �?              �?    �? �? - 维护连接池向 C++ 发送请�?         �?              �?    �? �? - 消息路由到各 Skynet 服务           �?              �?    �? └──────────────────────────────────────�?              �?    �?        �?                                              �?    └─────────┼───────────────────────────────────────────────�?              �?TCP (双向)
              �?    ┌─────────────────────────────────────────────────────────�?    �?                  C++ Gateway Server                     �?    └─────────────────────────────────────────────────────────�?    
    使用方式（Skynet 服务�?C++ 发请求）�?        local gateway = skynet.queryservice("gateway/cpp_gateway")
        
        -- 发送请求并等待响应
        local result = skynet.call(gateway, "lua", "request", msg_type, data)
        
        -- 快捷方法
        local player = skynet.call(gateway, "lua", "db_load_player", player_id)
        local ok = skynet.call(gateway, "lua", "verify_token", player_id, token)
]]

local skynet = require "skynet"
local socket = require "skynet.socket"
local rpc = require "rpc"

local CMD = {}

-- ==================== 配置 ====================

local CONFIG = {
    -- 监听配置（C++ 连接 Skynet�?    listen = {
        host = "0.0.0.0",
        port = 8888,
    },
    
    -- 主动连接配置（Skynet 连接 C++�?    cpp_server = {
        host = "127.0.0.1",
        port = 8889,
    },
    
    -- 连接池大小（主动连接 C++ 时使用）
    pool_size = 3,
    
    -- 请求超时（毫秒）
    timeout = 5000,
    
    -- 重连间隔（秒�?    reconnect_interval = 5,
}

-- ==================== 状态管�?====================

-- 被动连接（C++ 连接�?Skynet�?local passive_connections = {}   -- fd -> connection_info
local player_connections = {}    -- player_id -> fd

-- 主动连接池（Skynet 连接�?C++�?local active_pool = {
    connections = {},
    current = 1,
}

-- 请求等待队列
local pending_requests = {}      -- request_id -> {co, response_type, result, error, fd}
local request_counter = 0

-- ==================== 工具函数 ====================

local function next_request_id()
    request_counter = request_counter + 1
    if request_counter > 0x7FFFFFFF then
        request_counter = 1
    end
    return request_counter
end

-- 打包消息：[length(4)][type(2)][data]
local function pack_message(msg_type, binary)
    local length = 2 + #binary
    local header = string.char(
        (length >> 24) & 0xFF,
        (length >> 16) & 0xFF,
        (length >> 8) & 0xFF,
        length & 0xFF,
        (msg_type >> 8) & 0xFF,
        msg_type & 0xFF
    )
    return header .. binary
end

-- 解析消息�?local function parse_header(data)
    if #data < 6 then
        return nil, nil
    end
    
    local length = (string.byte(data, 1) << 24) | 
                   (string.byte(data, 2) << 16) | 
                   (string.byte(data, 3) << 8) | 
                   string.byte(data, 4)
    
    local msg_type = (string.byte(data, 5) << 8) | string.byte(data, 6)
    
    return length, msg_type
end

-- ============================================================
-- Part 1: 被动模式 - 处理来自 C++ 的请�?-- ============================================================

local function new_passive_connection(fd)
    return {
        fd = fd,
        player_id = nil,
        service = nil,
        last_active = skynet.time(),
        mode = "passive",
    }
end

-- 编码响应
local function encode_response(msg_type, response_data)
    local binary, err = rpc.encode(msg_type, response_data)
    if not binary then
        skynet.error("[Gateway] Failed to encode response:", err)
        return nil
    end
    return binary
end

-- 编码错误响应
local function encode_error(request_type, error_msg)
    local response_type = rpc.get_response_type(request_type) or (request_type + 100)
    local response = {
        success = false,
        message = error_msg or "unknown error",
    }
    return rpc.encode(response_type, response)
end

-- ==================== 消息处理器（C++ �?Skynet�?===================

-- 处理进入游戏
local function handle_enter_game(conn, req_data)
    local player_id = req_data.player_id
    local token = req_data.token
    
    skynet.error(string.format("[Gateway] Player entering: %s", player_id))
    
    -- TODO: 验证 token
    
    -- 请求 player_manager 创建/获取玩家服务
    local player_mgr = skynet.queryservice("player/player_manager")
    local player_service = skynet.call(player_mgr, "lua", "get_or_create_player", player_id)
    
    -- 保存连接信息
    conn.player_id = player_id
    conn.service = player_service
    player_connections[player_id] = conn.fd
    
    skynet.error(string.format("[Gateway] Player %s entered, service: %s", 
        player_id, skynet.address(player_service)))
    
    -- 获取玩家数据
    local player_data = skynet.call(player_service, "lua", "get_data")
    
    -- 构造响�?    local response = {
        success = true,
        message = "Enter game successfully",
        player_data = {
            player_id = player_data.player_id,
            level = player_data.level or 1,
            exp = player_data.exp or 0,
            gold = player_data.gold or 0,
            owned_cards = player_data.cards or {},
            achievements = {
                progress = player_data.achievements and player_data.achievements.progress or {},
                completed = player_data.achievements and player_data.achievements.completed or {},
            },
            last_login = player_data.last_login or 0,
        }
    }
    
    return encode_response(rpc.MSG.ENTER_GAME_RES, response)
end

-- 处理离开游戏
local function handle_leave_game(conn, req_data)
    local player_id = conn.player_id
    
    if not player_id then
        skynet.error("[Gateway] Leave game: player not logged in")
        return encode_error(rpc.MSG.LEAVE_GAME, "not_logged_in")
    end
    
    skynet.error(string.format("[Gateway] Player leaving: %s, reason: %s", 
        player_id, req_data.reason or "unknown"))
    
    -- 通知 player_manager
    local player_mgr = skynet.queryservice("player/player_manager")
    skynet.send(player_mgr, "lua", "player_offline", player_id)
    
    -- 清理连接
    player_connections[player_id] = nil
    conn.player_id = nil
    conn.service = nil
    
    return encode_response(rpc.MSG.LEAVE_GAME_RES, {
        success = true,
        message = "Leave game successfully",
    })
end

-- 处理玩家操作
local function handle_player_action(conn, req_data)
    local player_id = conn.player_id
    
    if not player_id or not conn.service then
        skynet.error("[Gateway] Action: player not logged in")
        return encode_error(rpc.MSG.PLAYER_ACTION, "not_logged_in")
    end
    
    skynet.error(string.format("[Gateway] Player %s action: %s", player_id, req_data.action_type))
    
    -- 转发到玩家服�?    local ok, result = pcall(skynet.call, conn.service, "lua", "action", 
        req_data.action_type, req_data.action_data)
    
    if not ok then
        skynet.error("[Gateway] Action failed:", result)
        return encode_error(rpc.MSG.PLAYER_ACTION, "action_failed: " .. tostring(result))
    end
    
    -- 解析结果
    local success = string.find(result or "", "action_ok") ~= nil
    local unlocked_achievements = {}
    
    local ach_start = string.find(result or "", "achievement:")
    if ach_start then
        local ach_str = string.sub(result, ach_start + 12)
        for ach_id in string.gmatch(ach_str, "([^,]+)") do
            table.insert(unlocked_achievements, {
                id = ach_id,
                name = "Achievement " .. ach_id,
                description = "Description for " .. ach_id,
                reward_gold = 100,
            })
        end
    end
    
    return encode_response(rpc.MSG.PLAYER_ACTION_RES, {
        success = success,
        message = result or "",
        result_data = "",
        unlocked_achievements = unlocked_achievements,
    })
end

-- 处理获取玩家状�?local function handle_get_player_status(conn, req_data)
    local player_id = req_data.player_id
    
    skynet.error(string.format("[Gateway] Get player status: %s", player_id))
    
    local player_mgr = skynet.queryservice("player/player_manager")
    local player_service = skynet.call(player_mgr, "lua", "get_player", player_id)
    
    if not player_service then
        return encode_error(rpc.MSG.GET_PLAYER_STATUS, "player_not_found")
    end
    
    local player_data = skynet.call(player_service, "lua", "get_data")
    
    return encode_response(rpc.MSG.GET_PLAYER_STATUS_RES, {
        success = true,
        player_data = {
            player_id = player_data.player_id,
            level = player_data.level or 1,
            exp = player_data.exp or 0,
            gold = player_data.gold or 0,
            owned_cards = player_data.cards or {},
            achievements = {
                progress = player_data.achievements and player_data.achievements.progress or {},
                completed = player_data.achievements and player_data.achievements.completed or {},
            },
            last_login = player_data.last_login or 0,
        }
    })
end

-- 消息处理器映�?local inbound_handlers = {
    [rpc.MSG.ENTER_GAME] = handle_enter_game,
    [rpc.MSG.LEAVE_GAME] = handle_leave_game,
    [rpc.MSG.PLAYER_ACTION] = handle_player_action,
    [rpc.MSG.GET_PLAYER_STATUS] = handle_get_player_status,
}

-- 处理来自 C++ 的消�?local function process_inbound_message(fd, msg_type, msg_data)
    skynet.error(string.format("[Gateway] Inbound: fd=%d, type=%d (%s)", 
        fd, msg_type, rpc.get_msg_name(msg_type)))
    
    local conn = passive_connections[fd]
    if not conn then
        skynet.error("[Gateway] Connection not found:", fd)
        return nil
    end
    
    conn.last_active = skynet.time()
    
    -- 解码消息
    local decoded_data, err = rpc.decode(msg_type, msg_data)
    if not decoded_data then
        skynet.error("[Gateway] Decode failed:", err)
        return encode_error(msg_type, "decode_error")
    end
    
    -- 查找处理�?    local handler = inbound_handlers[msg_type]
    if handler then
        local ok, result = pcall(handler, conn, decoded_data)
        if ok then
            return result
        else
            skynet.error("[Gateway] Handler error:", result)
            return encode_error(msg_type, "handler_error")
        end
    else
        skynet.error("[Gateway] Unknown inbound message type:", msg_type)
        return encode_error(msg_type, "unknown_message_type")
    end
end

-- 处理被动连接
local function handle_passive_connection(fd, addr)
    skynet.error(string.format("[Gateway] Passive connection: fd=%d, addr=%s", fd, addr))
    
    passive_connections[fd] = new_passive_connection(fd)
    socket.start(fd)
    
    local buffer = ""
    
    while true do
        local data = socket.read(fd)
        if not data then
            skynet.error("[Gateway] Passive connection closed:", fd)
            break
        end
        
        buffer = buffer .. data
        
        while #buffer >= 6 do
            local length, msg_type = parse_header(buffer)
            
            if not length or #buffer < 4 + length then
                break
            end
            
            local msg_data = buffer:sub(7, 4 + length)
            buffer = buffer:sub(5 + length)
            
            -- 处理消息
            local response_binary = process_inbound_message(fd, msg_type, msg_data)
            
            -- 发送响�?            if response_binary and #response_binary > 0 then
                local response_type = rpc.get_response_type(msg_type) or (msg_type + 100)
                socket.write(fd, pack_message(response_type, response_binary))
            end
        end
    end
    
    -- 清理
    local conn = passive_connections[fd]
    if conn then
        if conn.player_id then
            player_connections[conn.player_id] = nil
            local player_mgr = skynet.queryservice("player/player_manager")
            if player_mgr then
                skynet.send(player_mgr, "lua", "player_offline", conn.player_id)
            end
        end
        passive_connections[fd] = nil
    end
    socket.close(fd)
end

-- ============================================================
-- Part 2: 主动模式 - Skynet �?C++ 发送请�?-- ============================================================

-- 主动连接接收循环
local function active_receive_loop(conn)
    local buffer = ""
    
    while conn.connected do
        local data = socket.read(conn.fd)
        if not data then
            skynet.error("[Gateway] Active connection lost:", conn.fd)
            conn.connected = false
            
            -- 通知等待的请�?            for id, req in pairs(pending_requests) do
                if req.fd == conn.fd then
                    req.error = "connection lost"
                    skynet.wakeup(req.co)
                    pending_requests[id] = nil
                end
            end
            
            -- 延迟重连
            skynet.timeout(CONFIG.reconnect_interval * 100, function()
                if not conn.connected then
                    local new_fd = socket.open(CONFIG.cpp_server.host, CONFIG.cpp_server.port)
                    if new_fd then
                        conn.fd = new_fd
                        conn.connected = true
                        skynet.error("[Gateway] Active reconnected:", new_fd)
                        skynet.fork(function()
                            active_receive_loop(conn)
                        end)
                    end
                end
            end)
            break
        end
        
        conn.last_active = skynet.time()
        buffer = buffer .. data
        
        while #buffer >= 6 do
            local length, msg_type = parse_header(buffer)
            
            if not length or #buffer < 4 + length then
                break
            end
            
            local msg_data = buffer:sub(7, 4 + length)
            buffer = buffer:sub(5 + length)
            
            -- 解码
            local decoded, err = rpc.decode(msg_type, msg_data)
            if decoded then
                -- 匹配等待的请�?                local matched = false
                for id, req in pairs(pending_requests) do
                    if req.fd == conn.fd and req.response_type == msg_type then
                        req.result = decoded
                        skynet.wakeup(req.co)
                        pending_requests[id] = nil
                        matched = true
                        break
                    end
                end
                
                if not matched then
                    skynet.error("[Gateway] Unmatched response:", rpc.get_msg_name(msg_type))
                end
            else
                skynet.error("[Gateway] Decode error:", err)
            end
        end
    end
end

-- 创建主动连接
local function create_active_connection()
    local fd = socket.open(CONFIG.cpp_server.host, CONFIG.cpp_server.port)
    if not fd then
        skynet.error(string.format("[Gateway] Failed to connect C++ (%s:%d)", 
            CONFIG.cpp_server.host, CONFIG.cpp_server.port))
        return nil
    end
    
    local conn = {
        fd = fd,
        connected = true,
        last_active = skynet.time(),
        mode = "active",
    }
    
    skynet.fork(function()
        active_receive_loop(conn)
    end)
    
    skynet.error(string.format("[Gateway] Active connection created: fd=%d", fd))
    return conn
end

-- 初始化主动连接池
local function init_active_pool()
    for i = 1, CONFIG.pool_size do
        local conn = create_active_connection()
        if conn then
            table.insert(active_pool.connections, conn)
        end
    end
    
    skynet.error(string.format("[Gateway] Active pool initialized: %d connections", 
        #active_pool.connections))
    return #active_pool.connections > 0
end

-- 获取主动连接
local function get_active_connection()
    if #active_pool.connections == 0 then
        init_active_pool()
    end
    
    local start = active_pool.current
    repeat
        local conn = active_pool.connections[active_pool.current]
        active_pool.current = active_pool.current % #active_pool.connections + 1
        
        if conn and conn.connected then
            return conn
        end
    until active_pool.current == start
    
    -- 尝试重建
    for i, conn in ipairs(active_pool.connections) do
        if not conn.connected then
            local new_conn = create_active_connection()
            if new_conn then
                active_pool.connections[i] = new_conn
                return new_conn
            end
        end
    end
    
    return nil
end

-- 发送请求到 C++
local function send_to_cpp(msg_type, data, timeout_ms)
    local conn = get_active_connection()
    if not conn then
        return nil, "no connection available"
    end
    
    -- 编码
    local binary, err = rpc.encode(msg_type, data)
    if not binary then
        return nil, "encode failed: " .. tostring(err)
    end
    
    -- 发�?    local ok = pcall(socket.write, conn.fd, pack_message(msg_type, binary))
    if not ok then
        conn.connected = false
        return nil, "send failed"
    end
    
    -- 注册等待
    local request_id = next_request_id()
    local response_type = rpc.get_response_type(msg_type) or (msg_type + 100)
    
    local req = {
        co = coroutine.running(),
        fd = conn.fd,
        response_type = response_type,
        result = nil,
        error = nil,
    }
    pending_requests[request_id] = req
    
    -- 超时
    local timeout = timeout_ms or CONFIG.timeout
    skynet.timeout(timeout // 10, function()
        if pending_requests[request_id] then
            pending_requests[request_id] = nil
            req.error = "timeout"
            skynet.wakeup(req.co)
        end
    end)
    
    -- 等待响应
    skynet.wait(req.co)
    
    if req.error then
        return nil, req.error
    end
    return req.result
end

-- ============================================================
-- Part 3: 服务命令接口
-- ============================================================

-- 启动监听
function CMD.start()
    local host = skynet.getenv("cpp_gateway_host") or CONFIG.listen.host
    local port = tonumber(skynet.getenv("cpp_gateway_port")) or CONFIG.listen.port
    
    local listen_fd = socket.listen(host, port)
    skynet.error(string.format("[Gateway] Listening on %s:%d", host, port))
    
    socket.start(listen_fd, function(fd, addr)
        skynet.fork(handle_passive_connection, fd, addr)
    end)
end

-- ==================== Skynet �?C++ 请求接口 ====================

-- 通用请求
function CMD.request(msg_type, data, timeout_ms)
    return send_to_cpp(msg_type, data, timeout_ms)
end

-- 通用通知（不等待响应�?function CMD.notify(msg_type, data)
    local conn = get_active_connection()
    if not conn then
        return false, "no connection"
    end
    
    local binary, err = rpc.encode(msg_type, data)
    if not binary then
        return false, "encode failed"
    end
    
    local ok = pcall(socket.write, conn.fd, pack_message(msg_type, binary))
    return ok
end

-- ==================== 快捷方法：数据库 ====================

function CMD.db_load_player(player_id)
    -- 使用 DB_READ 消息通过 C++ Gateway 转发�?DBServer
    local result, err = send_to_cpp(rpc.MSG.DB_READ, {
        database = "poor_hearthstone",
        table = "player_data",
        query = {
            player_id = tostring(player_id),
        },
    })
    
    if result and result.success and result.results and #result.results > 0 then
        local row = result.results[1]
        if row and row.fields then
            return {
                player_id = row.fields.player_id,
                user_id = row.fields.user_id,
                nickname = row.fields.nickname,
                gold = tonumber(row.fields.gold) or 0,
                arcane_dust = tonumber(row.fields.arcane_dust) or 0,
                level = tonumber(row.fields.level) or 1,
                exp = tonumber(row.fields.exp) or 0,
                last_login = row.fields.last_login,
            }
        end
    end
    return nil, err or (result and result.message) or "player not found"
end

function CMD.db_save_player(player_id, data)
    -- 使用 DB_UPDATE 消息
    local result, err = send_to_cpp(rpc.MSG.DB_UPDATE, {
        database = "poor_hearthstone",
        table = "player_data",
        query = {
            player_id = tostring(player_id),
        },
        data = {
            nickname = data.nickname or "",
            gold = tostring(data.gold or 0),
            arcane_dust = tostring(data.arcane_dust or 0),
            last_login = os.date("%Y-%m-%d %H:%M:%S"),
        },
    })
    
    if result then
        return result.success, result.message
    end
    return false, err
end

function CMD.db_create_player(player_id, nickname)
    -- 使用 DB_CREATE 消息
    local result, err = send_to_cpp(rpc.MSG.DB_CREATE, {
        database = "poor_hearthstone",
        table = "player_data",
        data = {
            player_id = tostring(player_id),
            user_id = tostring(player_id),
            nickname = nickname or ("Player_" .. player_id),
            gold = "1000",
            arcane_dust = "0",
            last_login = os.date("%Y-%m-%d %H:%M:%S"),
        },
    })
    
    if result then
        return result.success, result.message
    end
    return false, err
end

-- 通用数据库请求接�?function CMD.db_request(request)
    -- request = {action, database, table, query, data}
    local msg_type
    if request.action == "create" then
        msg_type = rpc.MSG.DB_CREATE
    elseif request.action == "read" then
        msg_type = rpc.MSG.DB_READ
    elseif request.action == "update" then
        msg_type = rpc.MSG.DB_UPDATE
    elseif request.action == "delete" then
        msg_type = rpc.MSG.DB_DELETE
    else
        return nil, "unknown action: " .. tostring(request.action)
    end
    
    return send_to_cpp(msg_type, {
        database = request.database or "poor_hearthstone",
        table = request.table,
        query = request.query,
        data = request.data,
    })
end

-- ==================== 快捷方法：登录验�?====================

function CMD.verify_token(player_id, token)
    local result, err = send_to_cpp(rpc.MSG.ENTER_GAME, {
        player_id = tostring(player_id),
        token = token,
        client_version = "1.0.0",
    })
    
    if result then
        return result.success, result.player_data, result.message
    end
    return false, nil, err
end

-- ==================== C++ �?Skynet 推送接�?====================

function CMD.send_to_player(player_id, msg_type, data)
    local fd = player_connections[player_id]
    if not fd then
        return false, "player not connected"
    end
    
    local binary, err = rpc.encode(msg_type, data)
    if not binary then
        return false, "encode_error"
    end
    
    socket.write(fd, pack_message(msg_type, binary))
    return true
end

function CMD.broadcast(msg_type, data, exclude_player)
    local binary, err = rpc.encode(msg_type, data)
    if not binary then
        return false, "encode_error"
    end
    
    local msg = pack_message(msg_type, binary)
    local count = 0
    
    for player_id, fd in pairs(player_connections) do
        if player_id ~= exclude_player then
            socket.write(fd, msg)
            count = count + 1
        end
    end
    
    return true, count
end

-- ==================== 管理接口 ====================

function CMD.get_online_count()
    local count = 0
    for _ in pairs(player_connections) do
        count = count + 1
    end
    return count
end

function CMD.get_online_players()
    local players = {}
    for player_id, _ in pairs(player_connections) do
        table.insert(players, player_id)
    end
    return players
end

function CMD.kick_player(player_id, reason)
    local fd = player_connections[player_id]
    if not fd then
        return false, "player not connected"
    end
    
    local binary = rpc.encode(rpc.MSG.PUSH_NOTIFICATION, {
        player_id = player_id,
        notification_type = "kick",
        title = "Disconnected",
        content = reason or "You have been disconnected",
    })
    if binary then
        socket.write(fd, pack_message(rpc.MSG.PUSH_NOTIFICATION, binary))
    end
    
    socket.close(fd)
    return true
end

function CMD.info()
    local info = {
        passive_connections = 0,
        active_connections = 0,
        active_available = 0,
        online_players = 0,
        pending_requests = 0,
    }
    
    for _ in pairs(passive_connections) do
        info.passive_connections = info.passive_connections + 1
    end
    
    for _ in pairs(player_connections) do
        info.online_players = info.online_players + 1
    end
    
    info.active_connections = #active_pool.connections
    for _, conn in ipairs(active_pool.connections) do
        if conn.connected then
            info.active_available = info.active_available + 1
        end
    end
    
    for _ in pairs(pending_requests) do
        info.pending_requests = info.pending_requests + 1
    end
    
    return info
end

function CMD.set_config(key, value)
    if type(value) == "table" then
        CONFIG[key] = CONFIG[key] or {}
        for k, v in pairs(value) do
            CONFIG[key][k] = v
        end
    else
        CONFIG[key] = value
    end
    return true
end

-- ============================================================
-- 服务启动
-- ============================================================

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
            skynet.error("[Gateway] Unknown command:", cmd)
            if session ~= 0 then
                skynet.ret(skynet.pack(nil, "unknown command"))
            end
        end
    end)
    
    -- 初始�?RPC
    rpc.init()
    skynet.error("[Gateway] RPC initialized:", rpc.get_method())
    
    -- 启动监听
    CMD.start()
    
    -- 延迟初始化主动连接池
    skynet.fork(function()
        skynet.sleep(100)  -- 等待 C++ Gateway 启动
        init_active_pool()
    end)
    
    skynet.error("[Gateway] Service started (bidirectional)")
end)
