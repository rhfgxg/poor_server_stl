--[[
    数据库封装模块
    
    当前模式：Mock（内存模拟）
    生产模式：待实现内部 DB 代理服务
    
    安全说明：
        - 数据库操作不应通过公开的 Gateway gRPC 端口暴露
        - 生产环境应使用专门的内部服务或直连 DBServer
        - 当前使用 Mock 模式进行开发测试
    
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
]]

local skynet = require "skynet"

local M = {}
M.player = {}      -- 玩家数据接口
M.achievement = {} -- 成就数据接口

-- ==================== 状态 ====================

local initialized = false
local using_mock = true  -- 当前默认使用 Mock 模式
local mock_storage = nil

-- ==================== Mock 数据库 ====================

local function mock_init()
    skynet.error("[DB] Using in-memory Mock database")
    skynet.error("[DB] WARNING: Data will NOT persist!")
    
    mock_storage = {
        player_data = {},
        player_achievements = {},
        player_collection = {},
    }
    using_mock = true
    return true
end

local function mock_read(table_name, player_id)
    if not mock_storage[table_name] then
        return nil
    end
    return mock_storage[table_name][tostring(player_id)]
end

local function mock_write(table_name, player_id, data)
    if not mock_storage[table_name] then
        mock_storage[table_name] = {}
    end
    mock_storage[table_name][tostring(player_id)] = data
    return true
end

-- ==================== 通过 Gateway 访问 ====================

-- 获取 gateway 服务
local function get_gateway()
    if not gateway_service then
        gateway_service = skynet.queryservice("cpp_gateway")
    end
    return gateway_service
end

-- 通用数据库请求（通过 gateway 转发）
local function db_request(action, player_id, data)
    if using_mock then
        return nil, "mock mode"
    end
    
    local gateway = get_gateway()
    if not gateway then
        skynet.error("[DB] cpp_gateway not available")
        return nil, "gateway not available"
    end
    
    -- 构造请求数据
    local request_data = {
        player_id = tostring(player_id),
        action_type = action,
        action_data = data or "",
    }
    
    -- 通过 gateway 发送请求到 C++ 服务器
    local ok, result = pcall(skynet.call, gateway, "lua", "db_request", request_data)
    
    if not ok then
        skynet.error("[DB] Request failed:", result)
        return nil, tostring(result)
    end
    
    return result
end

-- ==================== 初始化接口 ====================

--- 初始化数据库模块
--- @return boolean 是否成功
function M.init()
    if initialized then
        skynet.error("[DB] Already initialized")
        return true
    end
    
    skynet.error("[DB] Initializing (gateway mode)...")
    
    -- 尝试获取 gateway 服务
    local ok, gw = pcall(skynet.queryservice, "cpp_gateway")
    
    if ok and gw then
        gateway_service = gw
        skynet.error("[DB] Using cpp_gateway for database access")
        using_mock = false
        initialized = true
        return true
    else
        skynet.error("[DB] cpp_gateway not available, using Mock mode")
        mock_init()
        initialized = true
        return true
    end
end

--- 获取状态信息
function M.info()
    return {
        initialized = initialized,
        using_mock = using_mock,
        mode = using_mock and "mock" or "gateway",
    }
end

-- ==================== 玩家数据接口 ====================

--- 加载玩家数据
--- @param player_id number|string 玩家ID
--- @return table|nil 玩家数据
function M.player.load(player_id)
    player_id = tostring(player_id)
    
    if using_mock then
        local data = mock_read("player_data", player_id)
        if data then
            return data
        end
        -- 返回默认数据
        return nil
    end
    
    local gateway = get_gateway()
    if not gateway then
        return nil
    end
    
    -- 使用 gateway 的快捷方法
    local ok, result = pcall(skynet.call, gateway, "lua", "db_load_player", player_id)
    
    if ok and result then
        return result
    end
    
    skynet.error("[DB] Failed to load player:", player_id, result)
    return nil
end

--- 保存玩家数据
--- @param player_id number|string 玩家ID
--- @param data table 玩家数据
--- @return boolean 是否成功
function M.player.save(player_id, data)
    player_id = tostring(player_id)
    
    if using_mock then
        return mock_write("player_data", player_id, data)
    end
    
    local gateway = get_gateway()
    if not gateway then
        return false
    end
    
    local ok, success, msg = pcall(skynet.call, gateway, "lua", "db_save_player", player_id, data)
    
    if ok then
        return success
    end
    
    skynet.error("[DB] Failed to save player:", player_id, msg)
    return false
end

--- 创建新玩家
--- @param player_id number|string 玩家ID
--- @param user_id number|string 用户ID
--- @param nickname string 昵称
--- @return boolean 是否成功
function M.player.create(player_id, user_id, nickname)
    player_id = tostring(player_id)
    
    local data = {
        player_id = player_id,
        user_id = user_id or player_id,
        nickname = nickname or ("Player_" .. player_id),
        gold = 1000,        -- 初始金币
        arcane_dust = 0,
        level = 1,
        exp = 0,
        cards = {},
        last_login = os.time(),
    }
    
    if using_mock then
        return mock_write("player_data", player_id, data)
    end
    
    local gateway = get_gateway()
    if not gateway then
        return false
    end
    
    local ok, success, msg = pcall(skynet.call, gateway, "lua", "db_create_player", player_id, nickname)
    
    if ok then
        return success
    end
    
    skynet.error("[DB] Failed to create player:", player_id, msg)
    return false
end

--- 检查玩家是否存在
--- @param player_id number|string 玩家ID
--- @return boolean 是否存在
function M.player.exists(player_id)
    local data = M.player.load(player_id)
    return data ~= nil
end

--- 更新玩家字段
--- @param player_id number|string 玩家ID
--- @param fields table 要更新的字段
--- @return boolean 是否成功
function M.player.update(player_id, fields)
    -- 先加载，再合并保存
    local data = M.player.load(player_id)
    if not data then
        return false
    end
    
    for k, v in pairs(fields) do
        data[k] = v
    end
    
    return M.player.save(player_id, data)
end

-- ==================== 成就数据接口 ====================

--- 加载玩家成就数据
--- @param player_id number|string 玩家ID
--- @return table 成就数据 {progress = {}, completed = {}}
function M.achievement.load(player_id)
    player_id = tostring(player_id)
    
    if using_mock then
        local data = mock_read("player_achievements", player_id)
        if data then
            return data
        end
        return {progress = {}, completed = {}}
    end
    
    -- 通过玩家数据获取成就
    local player = M.player.load(player_id)
    if player and player.achievements then
        return player.achievements
    end
    
    return {progress = {}, completed = {}}
end

--- 保存玩家成就数据
--- @param player_id number|string 玩家ID
--- @param achievements table 成就数据
--- @return boolean 是否成功
function M.achievement.save(player_id, achievements)
    player_id = tostring(player_id)
    
    if using_mock then
        return mock_write("player_achievements", player_id, achievements)
    end
    
    -- 更新玩家数据中的成就字段
    return M.player.update(player_id, {achievements = achievements})
end

--- 更新成就进度
--- @param player_id number|string 玩家ID
--- @param achievement_id string 成就ID
--- @param progress number 进度值
--- @return boolean 是否成功
function M.achievement.update_progress(player_id, achievement_id, progress)
    local achievements = M.achievement.load(player_id)
    achievements.progress = achievements.progress or {}
    achievements.progress[achievement_id] = progress
    return M.achievement.save(player_id, achievements)
end

--- 完成成就
--- @param player_id number|string 玩家ID
--- @param achievement_id string 成就ID
--- @return boolean 是否成功
function M.achievement.complete(player_id, achievement_id)
    local achievements = M.achievement.load(player_id)
    achievements.completed = achievements.completed or {}
    achievements.completed[achievement_id] = os.time()
    -- 清除进度
    if achievements.progress then
        achievements.progress[achievement_id] = nil
    end
    return M.achievement.save(player_id, achievements)
end

--- 检查成就是否完成
--- @param player_id number|string 玩家ID
--- @param achievement_id string 成就ID
--- @return boolean 是否完成
function M.achievement.is_completed(player_id, achievement_id)
    local achievements = M.achievement.load(player_id)
    return achievements.completed and achievements.completed[achievement_id] ~= nil
end

-- ==================== 卡牌收藏接口 ====================

M.collection = {}

--- 加载玩家卡牌收藏
--- @param player_id number|string 玩家ID
--- @return table 卡牌列表 {card_id = count, ...}
function M.collection.load(player_id)
    player_id = tostring(player_id)
    
    if using_mock then
        local data = mock_read("player_collection", player_id)
        return data or {}
    end
    
    local player = M.player.load(player_id)
    if player and player.cards then
        return player.cards
    end
    
    return {}
end

--- 保存玩家卡牌收藏
--- @param player_id number|string 玩家ID
--- @param cards table 卡牌数据
--- @return boolean 是否成功
function M.collection.save(player_id, cards)
    player_id = tostring(player_id)
    
    if using_mock then
        return mock_write("player_collection", player_id, cards)
    end
    
    return M.player.update(player_id, {cards = cards})
end

--- 添加卡牌
--- @param player_id number|string 玩家ID
--- @param card_id string 卡牌ID
--- @param count number 数量（默认1）
--- @return boolean 是否成功
function M.collection.add_card(player_id, card_id, count)
    count = count or 1
    local cards = M.collection.load(player_id)
    cards[card_id] = (cards[card_id] or 0) + count
    return M.collection.save(player_id, cards)
end

-- ==================== 测试接口 ====================

--- 测试数据库连接
--- @return boolean 是否正常
function M.test()
    if using_mock then
        -- Mock 模式测试
        local test_id = "test_" .. os.time()
        mock_write("player_data", test_id, {test = true})
        local data = mock_read("player_data", test_id)
        mock_storage["player_data"][test_id] = nil
        return data and data.test == true
    end
    
    -- Gateway 模式测试
    local gateway = get_gateway()
    if not gateway then
        return false
    end
    
    local ok, info = pcall(skynet.call, gateway, "lua", "info")
    return ok and info ~= nil
end

return M
