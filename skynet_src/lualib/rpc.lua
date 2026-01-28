--[[
    RPC 通信模块
    统一处理 Skynet 与 C++ 服务器之间的 Protobuf 通信
    
    使用方式：
        local rpc = require "rpc"
        
        -- 初始化（启动时调用一次）
        rpc.init()
        
        -- 编码消息
        local binary = rpc.encode(rpc.MSG.ENTER_GAME, {player_id = "123", token = "abc"})
        
        -- 解码消息
        local data = rpc.decode(rpc.MSG.ENTER_GAME, binary)
        
        -- 打包发送（带长度和类型头）
        local packet = rpc.pack(rpc.MSG.ENTER_GAME, {player_id = "123"})
        
        -- 解包接收
        local msg_type, data = rpc.unpack(packet)
]]

local skynet = require "skynet"

local M = {}

-- ==================== Protobuf 库加载 ====================

local pb, protoc
local use_native_pb = false

local function load_pb_library()
    local ok1, pb_module = pcall(require, "pb")
    local ok2, protoc_module = pcall(require, "protoc")
    
    if ok1 and ok2 then
        pb = pb_module
        protoc = protoc_module
        use_native_pb = true
        return true
    end
    
    if not ok1 then
        skynet.error("[RPC] pb module not available:", pb_module)
    end
    if not ok2 then
        skynet.error("[RPC] protoc module not available:", protoc_module)
    end
    
    return false
end

-- ==================== 消息类型定义 ====================

-- 消息类型枚举（与 C++ skynet_messages.proto 保持一致）
M.MSG = {
    UNKNOWN = 0,
    
    -- 请求消息 (1-99)
    ENTER_GAME = 1,
    LEAVE_GAME = 2,
    PLAYER_ACTION = 3,
    GET_PLAYER_STATUS = 4,
    
    -- 数据库请求 (20-29)，与 C++ common.proto ServiceType 对应
    DB_CREATE = 20,
    DB_READ = 22,
    DB_UPDATE = 24,
    DB_DELETE = 26,
    
    -- 响应消息 (101-199)
    ENTER_GAME_RES = 101,
    LEAVE_GAME_RES = 102,
    PLAYER_ACTION_RES = 103,
    GET_PLAYER_STATUS_RES = 104,
    
    -- 数据库响应 (21-27)
    DB_CREATE_RES = 21,
    DB_READ_RES = 23,
    DB_UPDATE_RES = 25,
    DB_DELETE_RES = 27,
    
    -- 推送消息 (200-299)
    PUSH_GAME_STATE = 200,
    PUSH_NOTIFICATION = 201,
    PUSH_ACHIEVEMENT = 202,
    
    -- 炉石传说 (300-399)
    HEARTHSTONE_MATCH = 300,
    HEARTHSTONE_BATTLE = 301,
    HEARTHSTONE_DECK = 302,
    HEARTHSTONE_COLLECTION = 303,
}

-- 消息类型到 Proto 名称映射
local MSG_TO_PROTO = {
    [1] = "skynet_proto.EnterGameRequest",
    [2] = "skynet_proto.LeaveGameRequest",
    [3] = "skynet_proto.PlayerActionRequest",
    [4] = "skynet_proto.GetPlayerStatusRequest",
    
    -- 数据库消息
    [20] = "skynet_proto.DBCreateRequest",
    [22] = "skynet_proto.DBReadRequest",
    [24] = "skynet_proto.DBUpdateRequest",
    [26] = "skynet_proto.DBDeleteRequest",
    [21] = "skynet_proto.DBCreateResponse",
    [23] = "skynet_proto.DBReadResponse",
    [25] = "skynet_proto.DBUpdateResponse",
    [27] = "skynet_proto.DBDeleteResponse",
    
    [101] = "skynet_proto.EnterGameResponse",
    [102] = "skynet_proto.LeaveGameResponse",
    [103] = "skynet_proto.PlayerActionResponse",
    [104] = "skynet_proto.GetPlayerStatusResponse",
    
    [200] = "skynet_proto.PushGameState",
    [201] = "skynet_proto.PushNotification",
    [202] = "skynet_proto.PushAchievement",
    
    [300] = "skynet_proto.HearthstoneMatchRequest",
    [301] = "skynet_proto.HearthstoneBattleRequest",
}

-- ==================== 内嵌 Proto 定义 ====================

local EMBEDDED_PROTO = [[
    syntax = "proto3";
    package skynet_proto;
    
    message EnterGameRequest {
        string player_id = 1;
        string token = 2;
        string client_version = 3;
    }
    
    message EnterGameResponse {
        bool success = 1;
        string message = 2;
        PlayerData player_data = 3;
    }
    
    message LeaveGameRequest {
        string player_id = 1;
        string reason = 2;
    }
    
    message LeaveGameResponse {
        bool success = 1;
        string message = 2;
    }
    
    message PlayerActionRequest {
        string player_id = 1;
        string action_type = 2;
        bytes action_data = 3;
    }
    
    message PlayerActionResponse {
        bool success = 1;
        string message = 2;
        bytes result_data = 3;
        repeated Achievement unlocked_achievements = 4;
    }
    
    message GetPlayerStatusRequest {
        string player_id = 1;
    }
    
    message GetPlayerStatusResponse {
        bool success = 1;
        PlayerData player_data = 2;
    }
    
    message PlayerData {
        string player_id = 1;
        int32 level = 2;
        int32 exp = 3;
        int32 gold = 4;
        repeated string owned_cards = 5;
        AchievementData achievements = 6;
        int64 last_login = 7;
    }
    
    message AchievementData {
        map<string, int32> progress = 1;
        map<string, bool> completed = 2;
    }
    
    message Achievement {
        string id = 1;
        string name = 2;
        string description = 3;
        int32 reward_gold = 4;
    }
    
    message PushGameState {
        string player_id = 1;
        string state_type = 2;
        bytes state_data = 3;
    }
    
    message PushNotification {
        string player_id = 1;
        string notification_type = 2;
        string title = 3;
        string content = 4;
    }
    
    message PushAchievement {
        string player_id = 1;
        Achievement achievement = 2;
    }
    
    message HearthstoneMatchRequest {
        string player_id = 1;
        string deck_id = 2;
    }
    
    message HearthstoneMatchResponse {
        bool success = 1;
        string message = 2;
        string match_id = 3;
    }
    
    message HearthstoneBattleRequest {
        string player_id = 1;
        string battle_id = 2;
        string action = 3;
        bytes action_params = 4;
    }
    
    message HearthstoneBattleResponse {
        bool success = 1;
        string message = 2;
        bytes battle_state = 3;
    }
    
    // 数据库请求/响应 (与 C++ server_db.proto 保持一致)
    message DBCreateRequest {
        bytes database = 1;
        bytes table = 2;
        map<string, bytes> data = 3;
    }
    
    message DBCreateResponse {
        bool success = 1;
        bytes message = 2;
    }
    
    message DBReadRequest {
        bytes database = 1;
        bytes table = 2;
        map<string, bytes> query = 3;
    }
    
    message DBReadResponse {
        bool success = 1;
        bytes message = 2;
        repeated DBResult results = 3;
    }
    
    message DBResult {
        map<string, bytes> fields = 1;
    }
    
    message DBUpdateRequest {
        bytes database = 1;
        bytes table = 2;
        map<string, bytes> query = 3;
        map<string, bytes> data = 4;
    }
    
    message DBUpdateResponse {
        bool success = 1;
        bytes message = 2;
    }
    
    message DBDeleteRequest {
        bytes database = 1;
        bytes table = 2;
        map<string, bytes> query = 3;
    }
    
    message DBDeleteResponse {
        bool success = 1;
        bytes message = 2;
    }
]]

-- ==================== 简单序列化（降级方案）====================

local function simple_serialize(data)
    local parts = {}
    for k, v in pairs(data) do
        if v ~= nil then
            if type(v) == "boolean" then
                table.insert(parts, k .. ":" .. (v and "true" or "false"))
            elseif type(v) == "table" then
                -- 简单处理数组
                table.insert(parts, k .. ":[table]")
            else
                table.insert(parts, k .. ":" .. tostring(v))
            end
        end
    end
    return table.concat(parts, "|")
end

local function simple_deserialize(str)
    local data = {}
    for part in string.gmatch(str, "([^|]+)") do
        local key, value = string.match(part, "([^:]+):(.+)")
        if key and value then
            if value == "true" then
                data[key] = true
            elseif value == "false" then
                data[key] = false
            elseif tonumber(value) then
                data[key] = tonumber(value)
            else
                data[key] = value
            end
        end
    end
    return data
end

-- ==================== 初始化 ====================

local initialized = false

--- 初始化 RPC 模块
--- @return boolean 是否成功
function M.init()
    if initialized then
        return true
    end
    
    skynet.error("[RPC] Initializing...")
    
    -- 尝试加载原生 pb 库
    if load_pb_library() then
        skynet.error("[RPC] Using native lua-protobuf")
        
        -- 加载内嵌 Proto 定义
        local p = protoc.new()
        local ok, err = pcall(function()
            p:load(EMBEDDED_PROTO)
        end)
        
        if not ok then
            skynet.error("[RPC] Failed to load proto:", err)
            use_native_pb = false
        end
    end
    
    if not use_native_pb then
        skynet.error("[RPC] Using simple text serialization (fallback)")
        skynet.error("[RPC] WARNING: For production, please install lua-protobuf")
    end
    
    initialized = true
    skynet.error("[RPC] Initialized successfully")
    return true
end

-- ==================== 编解码接口 ====================

--- 编码消息
--- @param msg_type number 消息类型
--- @param data table 消息数据
--- @return string|nil 编码后的二进制数据
--- @return string|nil 错误信息
function M.encode(msg_type, data)
    if not initialized then
        M.init()
    end
    
    local proto_name = MSG_TO_PROTO[msg_type]
    if not proto_name then
        return nil, "unknown message type: " .. tostring(msg_type)
    end
    
    if use_native_pb then
        local ok, result = pcall(pb.encode, proto_name, data)
        if ok then
            return result
        end
        return nil, "encode failed: " .. tostring(result)
    else
        local ok, result = pcall(simple_serialize, data)
        if ok then
            return result
        end
        return nil, "serialize failed: " .. tostring(result)
    end
end

--- 解码消息
--- @param msg_type number 消息类型
--- @param binary string 二进制数据
--- @return table|nil 解码后的数据
--- @return string|nil 错误信息
function M.decode(msg_type, binary)
    if not initialized then
        M.init()
    end
    
    local proto_name = MSG_TO_PROTO[msg_type]
    if not proto_name then
        return nil, "unknown message type: " .. tostring(msg_type)
    end
    
    if use_native_pb then
        local ok, result = pcall(pb.decode, proto_name, binary)
        if ok then
            return result
        end
        return nil, "decode failed: " .. tostring(result)
    else
        local ok, result = pcall(simple_deserialize, binary)
        if ok then
            return result
        end
        return nil, "deserialize failed: " .. tostring(result)
    end
end

-- ==================== 打包/解包（带协议头）====================

--- 打包消息（用于 TCP 发送）
--- 格式：[4字节长度][2字节类型][数据]
--- @param msg_type number 消息类型
--- @param data table 消息数据
--- @return string|nil 打包后的数据
--- @return string|nil 错误信息
function M.pack(msg_type, data)
    local binary, err = M.encode(msg_type, data)
    if not binary then
        return nil, err
    end
    
    local total_len = 2 + #binary  -- 类型(2) + 数据
    local header = string.pack(">I4I2", total_len, msg_type)
    return header .. binary
end

--- 解包消息（从 TCP 接收）
--- @param packet string 完整的数据包（不含长度头）
--- @return number|nil 消息类型
--- @return table|nil 消息数据
--- @return string|nil 错误信息
function M.unpack(packet)
    if #packet < 2 then
        return nil, nil, "packet too short"
    end
    
    local msg_type = string.unpack(">I2", packet:sub(1, 2))
    local binary = packet:sub(3)
    
    local data, err = M.decode(msg_type, binary)
    if not data then
        return msg_type, nil, err
    end
    
    return msg_type, data
end

--- 读取消息长度（从 TCP 头部）
--- @param header string 4字节头部
--- @return number 消息长度
function M.read_length(header)
    if #header < 4 then
        return 0
    end
    return string.unpack(">I4", header)
end

-- ==================== 辅助函数 ====================

--- 判断是否为响应消息
function M.is_response(msg_type)
    return msg_type >= 100 and msg_type < 200
end

--- 判断是否为推送消息
function M.is_push(msg_type)
    return msg_type >= 200
end

--- 获取请求对应的响应类型
function M.get_response_type(request_type)
    -- DB 消息 (20,22,24,26) -> (21,23,25,27)
    if request_type == 20 or request_type == 22 or request_type == 24 or request_type == 26 then
        return request_type + 1
    end
    -- 普通游戏消息 (1-99) -> (101-199)
    if request_type >= 1 and request_type < 20 then
        return request_type + 100
    end
    return nil
end

--- 获取消息类型名称
function M.get_msg_name(msg_type)
    for name, value in pairs(M.MSG) do
        if value == msg_type then
            return name
        end
    end
    return "UNKNOWN"
end

--- 获取当前使用的序列化方式
function M.get_method()
    return use_native_pb and "protobuf" or "simple"
end

--- 打印消息（调试用）
function M.dump(msg_type, data)
    local name = M.get_msg_name(msg_type)
    skynet.error(string.format("[RPC] Message: %s (%d)", name, msg_type))
    if type(data) == "table" then
        for k, v in pairs(data) do
            skynet.error(string.format("  %s = %s", k, tostring(v)))
        end
    end
end

-- ==================== 测试 ====================

function M.test()
    skynet.error("[RPC] Running tests...")
    skynet.error("[RPC] Method:", M.get_method())
    
    local test_data = {
        player_id = "test_player_123",
        token = "test_token_abc",
        client_version = "1.0.0"
    }
    
    -- 测试编解码
    local binary, err = M.encode(M.MSG.ENTER_GAME, test_data)
    if not binary then
        skynet.error("[RPC] Test FAILED: encode error:", err)
        return false
    end
    
    local decoded, err = M.decode(M.MSG.ENTER_GAME, binary)
    if not decoded then
        skynet.error("[RPC] Test FAILED: decode error:", err)
        return false
    end
    
    if decoded.player_id ~= test_data.player_id then
        skynet.error("[RPC] Test FAILED: data mismatch")
        return false
    end
    
    -- 测试打包解包
    local packet, err = M.pack(M.MSG.ENTER_GAME, test_data)
    if not packet then
        skynet.error("[RPC] Test FAILED: pack error:", err)
        return false
    end
    
    local length = M.read_length(packet:sub(1, 4))
    local msg_type, data, err = M.unpack(packet:sub(5))
    
    if msg_type ~= M.MSG.ENTER_GAME or not data then
        skynet.error("[RPC] Test FAILED: unpack error:", err)
        return false
    end
    
    skynet.error("[RPC] Test PASSED!")
    return true
end

return M
