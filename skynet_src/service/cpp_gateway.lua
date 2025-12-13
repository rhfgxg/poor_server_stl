-- C++ Gateway Service
-- 负责接收来自 C++ Logic Server 的 TCP 连接
-- 并将消息路由到相应的 Skynet 服务

local skynet = require "skynet"
local socket = require "skynet.socket"
local proto_util = require "proto_util"

local CMD = {}
local connections = {}  -- fd -> connection_info
local player_connections = {}  -- player_id -> fd

-- 连接信息结构
local function new_connection(fd)
    return {
        fd = fd,
        player_id = nil,
        service = nil,  -- 玩家对应的服务地址
        last_active = skynet.time()
    }
end

-- 处理来自 C++ 的消息
local function process_message(fd, msg_type, data)
    skynet.error(string.format("Received message from C++: fd=%d, type=%d, data_len=%d", 
        fd, msg_type, #data))
    
    local conn = connections[fd]
    if not conn then
        skynet.error("Connection not found:", fd)
        return
    end
    
    conn.last_active = skynet.time()
    
    -- 解码 Protobuf 消息
    local decoded_data, err = proto_util.decode(msg_type, data)
    if not decoded_data then
        skynet.error("Failed to decode message:", err)
        return pack_error_response(msg_type, "decode_error: " .. tostring(err))
    end
    
    proto_util.print_message(msg_type, decoded_data)
    
    -- 根据消息类型分发
    if msg_type == proto_util.MessageType.MSG_ENTER_GAME then
        -- 进入游戏
        return handle_enter_game(conn, decoded_data)
        
    elseif msg_type == proto_util.MessageType.MSG_LEAVE_GAME then
        -- 离开游戏
        return handle_leave_game(conn, decoded_data)
        
    elseif msg_type == proto_util.MessageType.MSG_PLAYER_ACTION then
        -- 玩家操作
        return handle_player_action(conn, decoded_data)
        
    elseif msg_type == proto_util.MessageType.MSG_GET_PLAYER_STATUS then
        -- 获取玩家状态
        return handle_get_player_status(conn, decoded_data)
        
    else
        skynet.error("Unknown message type:", msg_type)
        return pack_error_response(msg_type, "unknown_message_type")
    end
end

-- ==================== 消息处理函数 ====================

-- 处理进入游戏
local function handle_enter_game(conn, req_data)
    local player_id = req_data.player_id
    local token = req_data.token
    
    -- TODO: 验证 token
    
    -- 请求 player_manager 创建/获取玩家服务
    local player_mgr = skynet.queryservice("player_manager")
    local player_service = skynet.call(player_mgr, "lua", "get_or_create_player", player_id)
    
    -- 保存连接信息
    conn.player_id = player_id
    conn.service = player_service
    player_connections[player_id] = conn.fd
    
    skynet.error(string.format("Player %s entered game, service: %s", 
        player_id, skynet.address(player_service)))
    
    -- 获取玩家数据
    local player_data = skynet.call(player_service, "lua", "get_data")
    
    -- 构造响应
    local response = {
        success = true,
        message = "Enter game successfully",
        player_data = {
            player_id = player_data.player_id,
            level = player_data.level,
            exp = player_data.exp,
            gold = player_data.gold,
            owned_cards = player_data.cards or {},
            achievements = {
                progress = player_data.achievements.progress or {},
                completed = player_data.achievements.completed or {}
            },
            last_login = player_data.last_login
        }
    }
    
    return pack_response(proto_util.MessageType.MSG_ENTER_GAME_RES, response)
end

-- 处理离开游戏
local function handle_leave_game(conn, req_data)
    local player_id = conn.player_id
    
    if not player_id then
        skynet.error("Player not logged in")
        return pack_error_response(proto_util.MessageType.MSG_LEAVE_GAME_RES, "not_logged_in")
    end
    
    -- 通知 player_manager
    local player_mgr = skynet.queryservice("player_manager")
    skynet.send(player_mgr, "lua", "player_offline", player_id)
    
    -- 清理连接
    player_connections[player_id] = nil
    conn.player_id = nil
    conn.service = nil
    
    skynet.error(string.format("Player %s left game, reason: %s", player_id, req_data.reason or "unknown"))
    
    -- 构造响应
    local response = {
        success = true,
        message = "Leave game successfully"
    }
    
    return pack_response(proto_util.MessageType.MSG_LEAVE_GAME_RES, response)
end

-- 处理玩家操作
local function handle_player_action(conn, req_data)
    local player_id = conn.player_id
    
    if not player_id or not conn.service then
        skynet.error("Player not logged in")
        return pack_error_response(proto_util.MessageType.MSG_PLAYER_ACTION_RES, "not_logged_in")
    end
    
    skynet.error(string.format("Player %s action: %s", player_id, req_data.action_type))
    
    -- 转发到玩家服务
    local ok, result = pcall(skynet.call, conn.service, "lua", "action", req_data.action_type, req_data.action_data)
    
    if not ok then
        skynet.error("Action failed:", result)
        return pack_error_response(proto_util.MessageType.MSG_PLAYER_ACTION_RES, "action_failed: " .. tostring(result))
    end
    
    -- 解析结果（假设 result 是字符串格式：action_ok:xxx|achievement:1,2,3）
    local success = string.find(result, "action_ok") ~= nil
    local message = result
    local unlocked_achievements = {}
    
    -- 解析成就
    local ach_start = string.find(result, "achievement:")
    if ach_start then
        local ach_str = string.sub(result, ach_start + 12)
        for ach_id in string.gmatch(ach_str, "([^,]+)") do
            table.insert(unlocked_achievements, {
                id = ach_id,
                name = "Achievement " .. ach_id,
                description = "Description for achievement " .. ach_id,
                reward_gold = 100
            })
        end
    end
    
    -- 构造响应
    local response = {
        success = success,
        message = message,
        result_data = "",
        unlocked_achievements = unlocked_achievements
    }
    
    return pack_response(proto_util.MessageType.MSG_PLAYER_ACTION_RES, response)
end

-- 处理获取玩家状态
local function handle_get_player_status(conn, req_data)
    local player_id = req_data.player_id
    
    -- 查找玩家服务
    local player_mgr = skynet.queryservice("player_manager")
    local player_service = skynet.call(player_mgr, "lua", "get_player", player_id)
    
    if not player_service then
        return pack_error_response(proto_util.MessageType.MSG_GET_PLAYER_STATUS_RES, "player_not_found")
    end
    
    -- 获取玩家数据
    local player_data = skynet.call(player_service, "lua", "get_data")
    
    -- 构造响应
    local response = {
        success = true,
        player_data = {
            player_id = player_data.player_id,
            level = player_data.level,
            exp = player_data.exp,
            gold = player_data.gold,
            owned_cards = player_data.cards or {},
            achievements = {
                progress = player_data.achievements.progress or {},
                completed = player_data.achievements.completed or {}
            },
            last_login = player_data.last_login
        }
    }
    
    return pack_response(proto_util.MessageType.MSG_GET_PLAYER_STATUS_RES, response)
end

-- ==================== 辅助函数 ====================

-- 打包响应消息
local function pack_response(msg_type, response_data)
    local binary, err = proto_util.encode(msg_type, response_data)
    if not binary then
        skynet.error("Failed to encode response:", err)
        return pack_error_response(msg_type, "encode_error")
    end
    return binary
end

-- 打包错误响应
local function pack_error_response(request_type, error_msg)
    local response_type = proto_util.get_response_type(request_type)
    if not response_type then
        response_type = request_type + 100
    end
    
    local response = {
        success = false,
        message = error_msg
    }
    
    local binary, err = proto_util.encode(response_type, response)
    if not binary then
        skynet.error("Failed to encode error response:", err)
        return ""
    end
    return binary
end

-- ==================== 协议处理 ====================

-- 解析消息协议：[length(4)][type(2)][data]
local function parse_message(data)
    if #data < 6 then
        return nil, nil, nil
    end
    
    local length = (string.byte(data, 1) << 24) | 
                   (string.byte(data, 2) << 16) | 
                   (string.byte(data, 3) << 8) | 
                   string.byte(data, 4)
    
    local msg_type = (string.byte(data, 5) << 8) | string.byte(data, 6)
    
    local msg_data = string.sub(data, 7, 6 + length - 2)
    
    return length, msg_type, msg_data
end

-- 打包响应消息：[length(4)][type(2)][data]
local function pack_message(msg_type, data)
    local length = 2 + #data  -- type(2) + data
    local header = string.char(
        (length >> 24) & 0xFF,
        (length >> 16) & 0xFF,
        (length >> 8) & 0xFF,
        length & 0xFF,
        (msg_type >> 8) & 0xFF,
        msg_type & 0xFF
    )
    return header .. data
end

-- 处理客户端连接
local function handle_connection(fd, addr)
    skynet.error(string.format("New C++ connection: fd=%d, addr=%s", fd, addr))
    
    connections[fd] = new_connection(fd)
    socket.start(fd)
    
    local buffer = ""
    
    while true do
        local data = socket.read(fd)
        if not data then
            -- 连接断开
            skynet.error("C++ connection closed:", fd)
            break
        end
        
        buffer = buffer .. data
        
        -- 解析消息
        while #buffer >= 6 do
            local length, msg_type, msg_data = parse_message(buffer)
            
            if not length then
                break
            end
            
            if #buffer < 6 + length - 2 then
                -- 数据不完整，等待更多数据
                break
            end
            
            -- 处理消息（返回已编码的 Protobuf 二进制数据）
            local response_binary = process_message(fd, msg_type, msg_data)
            
            -- 发送响应
            if response_binary and #response_binary > 0 then
                -- 获取响应消息类型
                local response_type = proto_util.get_response_type(msg_type) or (msg_type + 100)
                local response_msg = pack_message(response_type, response_binary)
                socket.write(fd, response_msg)
            end
            
            -- 移除已处理的数据
            buffer = string.sub(buffer, 6 + length - 2 + 1)
        end
    end
    
    -- 清理连接
    local conn = connections[fd]
    if conn and conn.player_id then
        player_connections[conn.player_id] = nil
    end
    connections[fd] = nil
    socket.close(fd)
end

-- 服务命令
function CMD.start()
    local host = skynet.getenv("cpp_gateway_host") or "0.0.0.0"
    local port = tonumber(skynet.getenv("cpp_gateway_port")) or 8888
    
    local listen_fd = socket.listen(host, port)
    skynet.error(string.format("C++ Gateway listening on %s:%d", host, port))
    
    socket.start(listen_fd, function(fd, addr)
        skynet.fork(handle_connection, fd, addr)
    end)
end

function CMD.send_to_player(player_id, msg_type, data)
    local fd = player_connections[player_id]
    if not fd then
        return false, "player not connected"
    end
    
    -- 编码推送消息
    local binary, err = proto_util.encode(msg_type, data)
    if not binary then
        skynet.error("Failed to encode push message:", err)
        return false, "encode_error"
    end
    
    local msg = pack_message(msg_type, binary)
    socket.write(fd, msg)
    return true
end

-- 启动服务
skynet.start(function()
    skynet.dispatch("lua", function(session, source, cmd, ...)
        local f = CMD[cmd]
        if f then
            if session == 0 then
                -- send 不需要回复
                f(...)
            else
                -- call 需要回复
                skynet.ret(skynet.pack(f(...)))
            end
        else
            skynet.error("Unknown command:", cmd)
        end
    end)
    
    -- 初始化 Protobuf
    skynet.error("Initializing Protobuf...")
    local ok, err = pcall(proto_util.load_proto)
    if not ok then
        skynet.error("Failed to load proto:", err)
        skynet.error("Will try to use embedded proto definitions")
    end
    
    -- 运行测试（可选，用于验证）
    -- proto_util.test()
    
    -- 自动启动监听
    CMD.start()
end)
