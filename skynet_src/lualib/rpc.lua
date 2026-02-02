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

-- 消息类型枚举（与 Skynet proto 文件保持一致）
M.MSG = {
    UNKNOWN = 0,
    
    -- ============ 会话消息 (Session) ============
    -- 请求消息 (1-99)
    ENTER_GAME = 1,
    LEAVE_GAME = 2,
    PLAYER_ACTION = 3,
    GET_PLAYER_STATUS = 4,
    HEARTBEAT = 5,
    
    -- 响应消息 (101-199)
    ENTER_GAME_RES = 101,
    LEAVE_GAME_RES = 102,
    PLAYER_ACTION_RES = 103,
    GET_PLAYER_STATUS_RES = 104,
    HEARTBEAT_RES = 105,
    
    -- 会话推送 (200-299)
    PUSH_GAME_STATE = 200,
    PUSH_NOTIFICATION = 201,
    PUSH_ACHIEVEMENT = 202,
    PUSH_KICK = 203,
    
    -- ============ 数据库消息 (DB) ============
    -- 请求 (20-29)，与 C++ common.proto ServiceType 对应
    DB_CREATE = 20,
    DB_READ = 22,
    DB_UPDATE = 24,
    DB_DELETE = 26,
    -- 响应 (21-27)
    DB_CREATE_RES = 21,
    DB_READ_RES = 23,
    DB_UPDATE_RES = 25,
    DB_DELETE_RES = 27,
    
    -- ============ 匹配消息 (Match) ============
    -- 请求 (300-349)
    START_MATCH = 300,
    CANCEL_MATCH = 301,
    GET_MATCH_STATUS = 302,
    -- 响应 (400-449)
    START_MATCH_RES = 400,
    CANCEL_MATCH_RES = 401,
    GET_MATCH_STATUS_RES = 402,
    -- 推送 (450-499)
    PUSH_MATCH_FOUND = 450,
    PUSH_MATCH_PROGRESS = 451,
    PUSH_MATCH_CANCELLED = 452,
    
    -- ============ 战斗消息 (Battle) ============
    -- 请求 (500-549)
    BATTLE_ACTION = 500,
    GET_BATTLE_STATE = 501,
    RECONNECT_BATTLE = 502,
    -- 响应 (600-649)
    BATTLE_ACTION_RES = 600,
    GET_BATTLE_STATE_RES = 601,
    RECONNECT_BATTLE_RES = 602,
    -- 推送 (650-699)
    PUSH_BATTLE_STATE_UPDATE = 650,
    PUSH_TURN_START = 651,
    PUSH_BATTLE_END = 652,
    PUSH_OPPONENT_ACTION = 653,
    
    -- ============ 收藏消息 (Collection) ============
    -- 请求 (700-749)
    GET_COLLECTION = 700,
    DISENCHANT_CARD = 701,
    CRAFT_CARD = 702,
    -- 响应 (800-849)
    GET_COLLECTION_RES = 800,
    DISENCHANT_CARD_RES = 801,
    CRAFT_CARD_RES = 802,
    
    -- ============ 卡组消息 (Deck) ============
    -- 请求 (750-799)
    GET_DECKS = 750,
    CREATE_DECK = 751,
    UPDATE_DECK = 752,
    DELETE_DECK = 753,
    IMPORT_DECK = 754,
    EXPORT_DECK = 755,
    -- 响应 (850-899)
    GET_DECKS_RES = 850,
    CREATE_DECK_RES = 851,
    UPDATE_DECK_RES = 852,
    DELETE_DECK_RES = 853,
    IMPORT_DECK_RES = 854,
    EXPORT_DECK_RES = 855,
    
    -- ============ 测试消息 ============
    ECHO = 50,
    ECHO_RES = 51,
}

-- 消息类型到 Proto 名称映射
local MSG_TO_PROTO = {
    -- ============ 会话消息 ============
    [1] = "skynet_proto.EnterGameRequest",
    [2] = "skynet_proto.LeaveGameRequest",
    [3] = "skynet_proto.PlayerActionRequest",
    [4] = "skynet_proto.GetPlayerStatusRequest",
    [5] = "skynet_proto.HeartbeatRequest",
    [101] = "skynet_proto.EnterGameResponse",
    [102] = "skynet_proto.LeaveGameResponse",
    [103] = "skynet_proto.PlayerActionResponse",
    [104] = "skynet_proto.GetPlayerStatusResponse",
    [105] = "skynet_proto.HeartbeatResponse",
    [200] = "skynet_proto.PushGameState",
    [201] = "skynet_proto.PushNotification",
    [202] = "skynet_proto.PushAchievement",
    [203] = "skynet_proto.PushKick",
    
    -- ============ 数据库消息 ============
    [20] = "skynet_proto.DBCreateRequest",
    [22] = "skynet_proto.DBReadRequest",
    [24] = "skynet_proto.DBUpdateRequest",
    [26] = "skynet_proto.DBDeleteRequest",
    [21] = "skynet_proto.DBCreateResponse",
    [23] = "skynet_proto.DBReadResponse",
    [25] = "skynet_proto.DBUpdateResponse",
    [27] = "skynet_proto.DBDeleteResponse",
    
    -- ============ 匹配消息 ============
    [300] = "skynet_proto.StartMatchRequest",
    [301] = "skynet_proto.CancelMatchRequest",
    [302] = "skynet_proto.GetMatchStatusRequest",
    [400] = "skynet_proto.StartMatchResponse",
    [401] = "skynet_proto.CancelMatchResponse",
    [402] = "skynet_proto.GetMatchStatusResponse",
    [450] = "skynet_proto.PushMatchFound",
    [451] = "skynet_proto.PushMatchProgress",
    [452] = "skynet_proto.PushMatchCancelled",
    
    -- ============ 战斗消息 ============
    [500] = "skynet_proto.BattleActionRequest",
    [501] = "skynet_proto.GetBattleStateRequest",
    [502] = "skynet_proto.ReconnectBattleRequest",
    [600] = "skynet_proto.BattleActionResponse",
    [601] = "skynet_proto.GetBattleStateResponse",
    [602] = "skynet_proto.ReconnectBattleResponse",
    [650] = "skynet_proto.PushBattleStateUpdate",
    [651] = "skynet_proto.PushTurnStart",
    [652] = "skynet_proto.PushBattleEnd",
    [653] = "skynet_proto.PushOpponentAction",
    
    -- ============ 收藏消息 ============
    [700] = "skynet_proto.GetCollectionRequest",
    [701] = "skynet_proto.DisenchantCardRequest",
    [702] = "skynet_proto.CraftCardRequest",
    [800] = "skynet_proto.GetCollectionResponse",
    [801] = "skynet_proto.DisenchantCardResponse",
    [802] = "skynet_proto.CraftCardResponse",
    
    -- ============ 卡组消息 ============
    [750] = "skynet_proto.GetDecksRequest",
    [751] = "skynet_proto.CreateDeckRequest",
    [752] = "skynet_proto.UpdateDeckRequest",
    [753] = "skynet_proto.DeleteDeckRequest",
    [754] = "skynet_proto.ImportDeckRequest",
    [755] = "skynet_proto.ExportDeckRequest",
    [850] = "skynet_proto.GetDecksResponse",
    [851] = "skynet_proto.CreateDeckResponse",
    [852] = "skynet_proto.UpdateDeckResponse",
    [853] = "skynet_proto.DeleteDeckResponse",
    [854] = "skynet_proto.ImportDeckResponse",
    [855] = "skynet_proto.ExportDeckResponse",
    
    -- ============ 测试消息 ============
    [50] = nil,  -- ECHO - 原始字符串
    [51] = nil,  -- ECHO_RES - 原始字符串
}

-- Proto 文件路径配置
local PROTO_DIR = "./protobuf/skynet/"
local PROTO_FILES = {
    "skynet_common.proto",
    "skynet_db.proto",
    "skynet_session.proto",
    "skynet_match.proto",
    "skynet_battle.proto",
    "skynet_collection.proto",
}

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

--- 读取 Proto 文件内容
local function read_proto_file(filepath)
    local f = io.open(filepath, "r")
    if not f then
        return nil, "cannot open file: " .. filepath
    end
    local content = f:read("*a")
    f:close()
    return content
end

--- 加载所有 Proto 文件
local function load_proto_files()
    local p = protoc.new()
    
    -- 设置 proto 文件搜索路径（用于 import）
    p:addpath(PROTO_DIR)
    
    -- 按顺序加载（common 需要先加载，因为其他文件会 import 它）
    for _, filename in ipairs(PROTO_FILES) do
        local filepath = PROTO_DIR .. filename
        local content, err = read_proto_file(filepath)
        
        if not content then
            skynet.error("[RPC] Failed to read proto file:", err)
            return false
        end
        
        local ok, load_err = pcall(function()
            p:load(content, filename)
        end)
        
        if not ok then
            skynet.error("[RPC] Failed to load proto:", filename, load_err)
            return false
        end
        
        skynet.error("[RPC] Loaded proto file:", filename)
    end
    
    return true
end

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
        
        -- 从外部文件加载 Proto 定义
        if not load_proto_files() then
            skynet.error("[RPC] Failed to load proto files, falling back to simple serialization")
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
    -- 会话响应 (101-199)
    if msg_type >= 101 and msg_type < 200 then return true end
    -- DB 响应 (21,23,25,27)
    if msg_type == 21 or msg_type == 23 or msg_type == 25 or msg_type == 27 then return true end
    -- 匹配响应 (400-449)
    if msg_type >= 400 and msg_type < 450 then return true end
    -- 战斗响应 (600-649)
    if msg_type >= 600 and msg_type < 650 then return true end
    -- 收藏响应 (800-849)
    if msg_type >= 800 and msg_type < 850 then return true end
    -- 卡组响应 (850-899)
    if msg_type >= 850 and msg_type < 900 then return true end
    return false
end

--- 判断是否为推送消息
function M.is_push(msg_type)
    -- 会话推送 (200-299)
    if msg_type >= 200 and msg_type < 300 then return true end
    -- 匹配推送 (450-499)
    if msg_type >= 450 and msg_type < 500 then return true end
    -- 战斗推送 (650-699)
    if msg_type >= 650 and msg_type < 700 then return true end
    return false
end

--- 获取请求对应的响应类型
function M.get_response_type(request_type)
    -- DB 消息 (20,22,24,26) -> (21,23,25,27)
    if request_type == 20 or request_type == 22 or request_type == 24 or request_type == 26 then
        return request_type + 1
    end
    -- 会话消息 (1-99) -> (101-199)
    if request_type >= 1 and request_type < 20 then
        return request_type + 100
    end
    -- 匹配消息 (300-349) -> (400-449)
    if request_type >= 300 and request_type < 350 then
        return request_type + 100
    end
    -- 战斗消息 (500-549) -> (600-649)
    if request_type >= 500 and request_type < 550 then
        return request_type + 100
    end
    -- 收藏消息 (700-749) -> (800-849)
    if request_type >= 700 and request_type < 750 then
        return request_type + 100
    end
    -- 卡组消息 (750-799) -> (850-899)
    if request_type >= 750 and request_type < 800 then
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
