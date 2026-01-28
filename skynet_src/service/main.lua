--[[
    Skynet 主启动脚本
    
    模块说明：
    - rpc.lua   : Protobuf/RPC 通信模块
    - redis.lua : Redis 缓存模块
    - db.lua    : 数据库访问模块
]]

local skynet = require "skynet"

-- 新的模块化库
local rpc = require "rpc"
local redis = require "redis"
local db = require "db"

skynet.start(function()
skynet.error("============================================")
skynet.error("Skynet Logic Server Starting...")
skynet.error("============================================")
    
-- ==================== 初始化核心模块 ====================
    
-- 1. 初始化 RPC/Protobuf
skynet.error("\n[1/2] Initializing RPC module...")
if rpc.init() then
    skynet.error("[RPC] Initialized, method:", rpc.get_method())
else
    skynet.error("[RPC] WARNING: Initialization failed!")
end
    
-- 2. 初始化 Redis
skynet.error("\n[2/2] Initializing Redis module...")
if redis.init() then
    local info = redis.info()
    skynet.error("[Redis] Initialized, mode:", info.using_redis and "Redis" or "Fallback")
else
    skynet.error("[Redis] WARNING: Initialization failed!")
end
    
skynet.error("")  -- 空行分隔
    
-- ==================== 启动服务 ====================
    
-- 启动 C++ 双向网关（统一处理 Skynet ↔ C++ 通信）
-- 注意：必须先启动 gateway，db 模块才能工作
local cpp_gateway = skynet.uniqueservice("gateway/cpp_gateway")
skynet.error("[Service] C++ Gateway (bidirectional) started:", skynet.address(cpp_gateway))
    
-- 现在可以安全初始化数据库模块（gateway 已就绪）
skynet.error("\n[DB] Initializing Database module...")
if db.init() then
    local info = db.info()
    skynet.error("[DB] Initialized, gateway:", info.gateway_available and "Connected" or "Not available")
else
    skynet.error("[DB] WARNING: Initialization failed!")
end
    
-- 启动玩家管理器
local player_mgr = skynet.uniqueservice("player/player_manager")
skynet.error("[Service] Player Manager started:", skynet.address(player_mgr))
    
-- 启动成就服务
local achievement_srv = skynet.uniqueservice("logic/achievement_service")
skynet.error("[Service] Achievement Service started:", skynet.address(achievement_srv))
    
-- 启动游戏逻辑管理器（炉石传说）
local hearthstone_mgr = skynet.uniqueservice("logic/hearthstone_manager")
skynet.error("[Service] Hearthstone Manager started:", skynet.address(hearthstone_mgr))
    
    skynet.error("============================================")
    skynet.error("Skynet Logic Server Started Successfully")
    skynet.error("All services online and ready")
    skynet.error("============================================")
    
    -- 退出启动服务（不退出整个 Skynet）
    skynet.exit()
end)
