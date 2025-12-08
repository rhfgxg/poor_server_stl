-- C++ Gateway Service
-- 负责接收来自 C++ Logic Server 的 TCP 连接
-- 并将消息路由到相应的 Skynet 服务

local skynet = require "skynet"
local socket = require "skynet.socket"
local netpack = require "skynet.netpack"

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
    
    -- 根据消息类型分发
    if msg_type == 1 then
        -- ENTER_GAME
        local player_id = data  -- 简化：直接使用数据作为 player_id
        
        -- 请求 player_manager 创建/获取玩家服务
        local player_mgr = skynet.queryservice("player_manager")
        local player_service = skynet.call(player_mgr, "lua", "get_or_create_player", player_id)
        
        -- 保存连接信息
        conn.player_id = player_id
        conn.service = player_service
        player_connections[player_id] = fd
        
        skynet.error(string.format("Player %s entered game, service: %s", 
            player_id, skynet.address(player_service)))
        
        -- 返回响应
        local response = player_id .. "_service"  -- 简化：返回服务标识
        return response
        
    elseif msg_type == 2 then
        -- LEAVE_GAME
        local player_id = conn.player_id
        if player_id then
            -- 通知 player_manager
            local player_mgr = skynet.queryservice("player_manager")
            skynet.send(player_mgr, "lua", "player_offline", player_id)
            
            -- 清理连接
            player_connections[player_id] = nil
            conn.player_id = nil
            conn.service = nil
            
            skynet.error("Player left game:", player_id)
        end
        
    elseif msg_type == 3 then
        -- PLAYER_ACTION
        local player_id = conn.player_id
        if not player_id or not conn.service then
            skynet.error("Player not logged in")
            return "error: not logged in"
        end
        
        -- 转发到玩家服务
        local ok, result = pcall(skynet.call, conn.service, "lua", "action", data)
        if ok then
            return result
        else
            skynet.error("Action failed:", result)
            return "error: " .. tostring(result)
        end
        
    else
        skynet.error("Unknown message type:", msg_type)
        return "error: unknown message type"
    end
end

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
            
            -- 处理消息
            local response = process_message(fd, msg_type, msg_data)
            
            -- 发送响应
            if response then
                local response_msg = pack_message(msg_type + 100, response)  -- 响应类型 = 请求类型 + 100
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
    
    local msg = pack_message(msg_type, data)
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
    
    -- 自动启动监听
    CMD.start()
end)
