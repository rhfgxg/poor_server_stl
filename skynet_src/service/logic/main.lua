-- logic/main.lua
-- 逻辑服务器主服务
-- 职责：处理游戏逻辑，玩家数据管理，与数据库和网关服务器通信

local skynet = require "skynet"
local socket = require "skynet.socket"
local proto = require "proto"

local CMD = {}
local SOCKET = {}

-- 配置
local config = {
    gateway_host = "127.0.0.1",
    gateway_port = 8001,
    db_host = "127.0.0.1",
    db_port = 8002,
}

-- 连接管理
local connections = {
    gateway = nil,  -- 网关服务器连接
    db = nil,       -- 数据库服务器连接
}

-- 玩家会话管理
local player_sessions = {}  -- user_id -> session_info

-----------------------------------------------------------
-- Protobuf 消息收发辅助函数
-----------------------------------------------------------

-- 发送 protobuf 消息到 C++ 服务器
-- @param fd socket 连接句柄
-- @param msg_name string protobuf 消息类型
-- @param data table 消息数据
local function send_proto_msg(fd, msg_name, data)
    local body = proto.encode(msg_name, data)
    local len = #body
    -- 使用 4 字节大端序长度前缀
    local header = string.pack(">I4", len)
    socket.write(fd, header .. body)
    skynet.error(string.format("[Logic] Send: %s, size: %d", msg_name, len))
end

-- 接收 protobuf 消息
-- @param fd socket 连接句柄
-- @param msg_name string 期望的消息类型
-- @return table 解码后的消息，失败返回 nil
local function recv_proto_msg(fd, msg_name)
    -- 读取长度头（4字节）
    local header = socket.read(fd, 4)
    if not header then
        skynet.error("[Logic] Failed to read message header")
        return nil
    end
    
    local len = string.unpack(">I4", header)
    if len <= 0 or len > 1024 * 1024 then  -- 限制最大 1MB
        skynet.error(string.format("[Logic] Invalid message length: %d", len))
        return nil
    end
    
    -- 读取消息体
    local body = socket.read(fd, len)
    if not body then
        skynet.error("[Logic] Failed to read message body")
        return nil
    end
    
    -- 解码
    local ok, msg = pcall(proto.decode, msg_name, body)
    if not ok then
        skynet.error(string.format("[Logic] Failed to decode %s: %s", msg_name, msg))
        return nil
    end
    
    skynet.error(string.format("[Logic] Recv: %s, size: %d", msg_name, len))
    return msg
end

-----------------------------------------------------------
-- 与 C++ 服务器的连接管理
-----------------------------------------------------------

-- 连接到网关服务器
local function connect_to_gateway()
    skynet.error("[Logic] Connecting to gateway server...")
    local fd = socket.open(config.gateway_host, config.gateway_port)
    if not fd then
        skynet.error("[Logic] Failed to connect to gateway")
        return false
    end
    
    connections.gateway = fd
    skynet.error("[Logic] Connected to gateway server")
    
    -- 发送注册消息（假设有 RegisterLogicServer 消息）
    -- send_proto_msg(fd, "gateway.RegisterLogicServer", {
    --     server_id = 1,
    --     server_name = "logic_server_1"
    -- })
    
    return true
end

-- 连接到数据库服务器
local function connect_to_db()
    skynet.error("[Logic] Connecting to database server...")
    local fd = socket.open(config.db_host, config.db_port)
    if not fd then
        skynet.error("[Logic] Failed to connect to database")
        return false
    end
    
    connections.db = fd
    skynet.error("[Logic] Connected to database server")
    return true
end

-----------------------------------------------------------
-- 业务逻辑处理
-----------------------------------------------------------

-- 处理玩家登录
-- @param user_id number 用户ID
-- @param token string 登录令牌
local function handle_player_login(user_id, token)
    skynet.error(string.format("[Logic] Player login: user_id=%d, token=%s", user_id, token))
    
    -- TODO: 向数据库服务器查询玩家数据
    if connections.db then
        -- send_proto_msg(connections.db, "db.QueryPlayerRequest", {
        --     user_id = user_id
        -- })
        -- local player_data = recv_proto_msg(connections.db, "db.QueryPlayerResponse")
    end
    
    -- 创建玩家会话
    player_sessions[user_id] = {
        user_id = user_id,
        token = token,
        login_time = os.time(),
    }
    
    return {
        success = true,
        message = "Login success",
        user_id = user_id,
    }
end

-- 处理玩家登出
local function handle_player_logout(user_id)
    skynet.error(string.format("[Logic] Player logout: user_id=%d", user_id))
    
    -- TODO: 保存玩家数据到数据库
    if connections.db and player_sessions[user_id] then
        -- send_proto_msg(connections.db, "db.SavePlayerRequest", {
        --     user_id = user_id,
        --     player_data = player_sessions[user_id]
        -- })
    end
    
    player_sessions[user_id] = nil
    return { success = true }
end

-----------------------------------------------------------
-- Skynet 服务命令接口
-----------------------------------------------------------

-- 启动服务
function CMD.start()
    skynet.error("[Logic] Logic server starting...")
    
    -- 加载 protobuf descriptor
    local ok, err = proto.load_descriptor("protobuf/skynet/src/server_logic.pb")
    if not ok then
        skynet.error("[Logic] Failed to load logic proto:", err)
        -- 继续运行，但 proto 功能不可用
    end
    
    -- 可选：加载其他需要的 proto
    proto.load_descriptor("protobuf/skynet/src/common.pb")
    proto.load_descriptor("protobuf/skynet/src/server_gateway.pb")
    proto.load_descriptor("protobuf/skynet/src/server_db.pb")
    
    -- 连接到其他服务器（可选，根据实际架构）
    -- connect_to_gateway()
    -- connect_to_db()
    
    skynet.error("[Logic] Logic server started")
    return true
end

-- 停止服务
function CMD.stop()
    skynet.error("[Logic] Logic server stopping...")
    
    -- 断开所有连接
    if connections.gateway then
        socket.close(connections.gateway)
    end
    if connections.db then
        socket.close(connections.db)
    end
    
    skynet.error("[Logic] Logic server stopped")
    return true
end

-- 处理客户端消息（通过网关转发）
-- @param user_id number 用户ID
-- @param msg_type string 消息类型
-- @param msg_data string protobuf 编码的消息数据
function CMD.handle_client_message(user_id, msg_type, msg_data)
    skynet.error(string.format("[Logic] Handle client message: user_id=%d, type=%s", user_id, msg_type))
    
    -- 根据消息类型分发处理
    if msg_type == "logic.LoginRequest" then
        local req = proto.decode(msg_type, msg_data)
        local resp = handle_player_login(req.user_id, req.token)
        local resp_data = proto.encode("logic.LoginResponse", resp)
        return resp_data
    elseif msg_type == "logic.LogoutRequest" then
        local req = proto.decode(msg_type, msg_data)
        local resp = handle_player_logout(req.user_id)
        local resp_data = proto.encode("logic.LogoutResponse", resp)
        return resp_data
    else
        skynet.error("[Logic] Unknown message type:", msg_type)
        return nil
    end
end

-- 获取在线玩家数
function CMD.get_online_count()
    local count = 0
    for _ in pairs(player_sessions) do
        count = count + 1
    end
    return count
end

-----------------------------------------------------------
-- Skynet 服务启动
-----------------------------------------------------------

skynet.start(function()
    -- 注册 lua 协议的消息处理
    skynet.dispatch("lua", function(session, source, command, ...)
        local f = CMD[command]
        if f then
            if session == 0 then
                -- 不需要返回
                f(...)
            else
                -- 需要返回结果
                skynet.ret(skynet.pack(f(...)))
            end
        else
            skynet.error(string.format("[Logic] Unknown command: %s", command))
            if session ~= 0 then
                skynet.ret(skynet.pack(nil))
            end
        end
    end)
    
    -- 自动启动
    CMD.start()
end)
