-- Protobuf 支持库
-- 用于在 Skynet 中编解码 Protobuf 消息

local skynet = require "skynet"

-- 尝试加载 pb 和 protoc，如果失败则使用内嵌定义
local pb, protoc
local use_native_pb = false

local ok1, pb_module = pcall(require, "pb")
local ok2, protoc_module = pcall(require, "protoc")

if ok1 and ok2 then
    pb = pb_module
    protoc = protoc_module
    use_native_pb = true
    skynet.error("[ProtoUtil] Using native lua-protobuf library")
else
    skynet.error("[ProtoUtil] Native pb library not available, using pure Lua implementation")
    if not ok1 then
        skynet.error("[ProtoUtil] pb module load failed:", pb_module)
    end
    if not ok2 then
        skynet.error("[ProtoUtil] protoc module load failed:", protoc_module)
    end
    use_native_pb = false
end

local M = {}

-- ==================== 消息类型映射 ====================
M.MessageType = {
    MSG_UNKNOWN = 0,
    
    -- 玩家会话管理 (1-99)
    MSG_ENTER_GAME = 1,
    MSG_LEAVE_GAME = 2,
    MSG_PLAYER_ACTION = 3,
    MSG_GET_PLAYER_STATUS = 4,
    
    -- 响应消息 (101-199)
    MSG_ENTER_GAME_RES = 101,
    MSG_LEAVE_GAME_RES = 102,
    MSG_PLAYER_ACTION_RES = 103,
    MSG_GET_PLAYER_STATUS_RES = 104,
    
    -- 推送消息 (200-299)
    MSG_PUSH_GAME_STATE = 200,
    MSG_PUSH_NOTIFICATION = 201,
    MSG_PUSH_ACHIEVEMENT = 202,
    
    -- 炉石传说相关 (300-399)
    MSG_HEARTHSTONE_MATCH = 300,
    MSG_HEARTHSTONE_BATTLE = 301,
    MSG_HEARTHSTONE_DECK = 302,
    MSG_HEARTHSTONE_COLLECTION = 303,
}

-- 消息类型到 Proto 名称的映射
M.MessageTypeToProto = {
    [1] = "skynet_proto.EnterGameRequest",
    [2] = "skynet_proto.LeaveGameRequest",
    [3] = "skynet_proto.PlayerActionRequest",
    [4] = "skynet_proto.GetPlayerStatusRequest",
    
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

-- Proto 名称到消息类型的映射
M.ProtoToMessageType = {}
for msg_type, proto_name in pairs(M.MessageTypeToProto) do
    M.ProtoToMessageType[proto_name] = msg_type
end

-- ==================== 纯 Lua 序列化实现 ====================

-- 简单的字符串拼接序列化（用于降级方案）
local function serialize_simple(data)
    local parts = {}
    
    -- 按固定顺序序列化字段（根据消息类型）
    if data.player_id then
        table.insert(parts, "player_id:" .. tostring(data.player_id))
    end
    if data.token then
        table.insert(parts, "token:" .. tostring(data.token))
    end
    if data.client_version then
        table.insert(parts, "client_version:" .. tostring(data.client_version))
    end
    if data.success ~= nil then
        table.insert(parts, "success:" .. tostring(data.success))
    end
    if data.message then
        table.insert(parts, "message:" .. tostring(data.message))
    end
    if data.reason then
        table.insert(parts, "reason:" .. tostring(data.reason))
    end
    if data.action_type then
        table.insert(parts, "action_type:" .. tostring(data.action_type))
    end
    if data.level then
        table.insert(parts, "level:" .. tostring(data.level))
    end
    if data.exp then
        table.insert(parts, "exp:" .. tostring(data.exp))
    end
    if data.gold then
        table.insert(parts, "gold:" .. tostring(data.gold))
    end
    
    return table.concat(parts, "|")
end

-- 简单的反序列化
local function deserialize_simple(str)
    local data = {}
    
    for part in string.gmatch(str, "([^|]+)") do
        local key, value = string.match(part, "([^:]+):(.+)")
        if key and value then
            -- 类型转换
            if key == "success" then
                data[key] = (value == "true")
            elseif key == "level" or key == "exp" or key == "gold" then
                data[key] = tonumber(value)
            else
                data[key] = value
            end
        end
    end
    
    return data
end

-- ==================== 初始化 ====================

local proto_loaded = false

-- 加载 Proto 文件（使用原生库）
function M.load_proto_native()
    if not use_native_pb then
        return false, "Native pb library not available"
    end
    
    if proto_loaded then
        return true
    end
    
    local proto_path = "../../protobuf/cpp/skynet_messages.proto"
    local p = protoc.new()
    p.include_imports = true
    
    local ok, err = pcall(function()
        p:loadfile(proto_path)
    end)
    
    if not ok then
        skynet.error("[ProtoUtil] Failed to load proto file:", err)
        return false, err
    end
    
    proto_loaded = true
    skynet.error("[ProtoUtil] Proto loaded successfully from file (native)")
    return true
end

-- 加载内嵌 Proto 定义
function M.load_proto_embedded()
    if proto_loaded then
        return true
    end
    
    if use_native_pb then
        -- 如果有原生库，用它加载内嵌定义
        local p = protoc.new()
        p:load [[
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
        ]]
        proto_loaded = true
        skynet.error("[ProtoUtil] Proto loaded successfully (embedded with native pb)")
        return true
    else
        -- 完全没有原生库，使用纯 Lua 实现
        proto_loaded = true
        skynet.error("[ProtoUtil] Proto loaded successfully (pure Lua fallback - simple serialization)")
        skynet.error("[ProtoUtil] WARNING: Using simple text-based serialization, performance will be lower")
        skynet.error("[ProtoUtil] For production use, please install lua-protobuf")
        return true
    end
end

-- 统一加载入口
function M.load_proto()
    -- 优先尝试从文件加载
    local ok, err = M.load_proto_native()
    if ok then
        return true
    end
    
    -- 降级到内嵌定义
    skynet.error("[ProtoUtil] Falling back to embedded proto definitions")
    return M.load_proto_embedded()
end

-- ==================== 编解码函数 ====================

-- 使用原生 pb 编码
local function encode_native(proto_name, data)
    return pb.encode(proto_name, data)
end

-- 使用简单序列化编码（降级方案）
local function encode_simple(proto_name, data)
    return serialize_simple(data)
end

-- 使用原生 pb 解码
local function decode_native(proto_name, binary)
    return pb.decode(proto_name, binary)
end

-- 使用简单反序列化解码（降级方案）
local function decode_simple(proto_name, binary)
    return deserialize_simple(binary)
end

-- 编码消息
function M.encode(msg_type, data)
    if not proto_loaded then
        M.load_proto()
    end
    
    local proto_name = M.MessageTypeToProto[msg_type]
    if not proto_name then
        return nil, "Unknown message type: " .. tostring(msg_type)
    end
    
    local ok, result
    if use_native_pb then
        ok, result = pcall(encode_native, proto_name, data)
    else
        ok, result = pcall(encode_simple, proto_name, data)
    end
    
    if not ok then
        return nil, "Encode failed: " .. tostring(result)
    end
    
    return result
end

-- 解码消息
function M.decode(msg_type, binary)
    if not proto_loaded then
        M.load_proto()
    end
    
    local proto_name = M.MessageTypeToProto[msg_type]
    if not proto_name then
        return nil, "Unknown message type: " .. tostring(msg_type)
    end
    
    local ok, result
    if use_native_pb then
        ok, result = pcall(decode_native, proto_name, binary)
    else
        ok, result = pcall(decode_simple, proto_name, binary)
    end
    
    if not ok then
        return nil, "Decode failed: " .. tostring(result)
    end
    
    return result
end

-- ==================== 辅助函数 ====================

-- 打印消息（用于调试）
function M.print_message(msg_type, data)
    local proto_name = M.MessageTypeToProto[msg_type]
    if proto_name then
        skynet.error(string.format("[ProtoUtil] Message Type: %d (%s)", msg_type, proto_name))
    else
        skynet.error(string.format("[ProtoUtil] Message Type: %d (Unknown)", msg_type))
    end
    
    if type(data) == "table" then
        for k, v in pairs(data) do
            skynet.error(string.format("  %s = %s", k, tostring(v)))
        end
    end
end

-- 检查消息类型是否为响应
function M.is_response(msg_type)
    return msg_type >= 100 and msg_type < 200
end

-- 检查消息类型是否为推送
function M.is_push(msg_type)
    return msg_type >= 200
end

-- 获取请求对应的响应类型
function M.get_response_type(request_type)
    if request_type >= 1 and request_type < 100 then
        return request_type + 100
    end
    return nil
end

-- 获取当前使用的序列化方式
function M.get_serialization_method()
    if use_native_pb then
        return "protobuf (native)"
    else
        return "simple text (fallback)"
    end
end

-- ==================== 测试函数 ====================

-- 测试编解码
function M.test()
    skynet.error("[ProtoUtil] Running tests...")
    skynet.error("[ProtoUtil] Serialization method:", M.get_serialization_method())
    
    -- 测试 EnterGameRequest
    local req_data = {
        player_id = "player_12345",
        token = "abc123xyz",
        client_version = "1.0.0"
    }
    
    local binary, err = M.encode(M.MessageType.MSG_ENTER_GAME, req_data)
    if not binary then
        skynet.error("[ProtoUtil] Test FAILED: Encode error:", err)
        return false
    end
    
    skynet.error("[ProtoUtil] Encoded data length:", #binary)
    skynet.error("[ProtoUtil] Encoded data:", binary)
    
    local decoded, err = M.decode(M.MessageType.MSG_ENTER_GAME, binary)
    if not decoded then
        skynet.error("[ProtoUtil] Test FAILED: Decode error:", err)
        return false
    end
    
    skynet.error("[ProtoUtil] Decoded data:")
    M.print_message(M.MessageType.MSG_ENTER_GAME, decoded)
    
    -- 验证数据
    if decoded.player_id == req_data.player_id and 
       decoded.token == req_data.token and 
       decoded.client_version == req_data.client_version then
        skynet.error("[ProtoUtil] Test PASSED!")
        return true
    else
        skynet.error("[ProtoUtil] Test FAILED: Data mismatch")
        return false
    end
end

return M
