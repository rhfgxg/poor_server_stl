--[[
    Skynet 主启动脚本 (简化测试版)
    
    保留模块：
    - rpc.lua   : Protobuf/RPC 通信模块
    - redis.lua : Redis 缓存模块
    - db.lua    : 数据库访问模块
    
    保留服务：
    - gateway/cpp_gateway : C++ 双向网关
    - logic/logic_service : 逻辑服务（测试用）
]]

local skynet = require "skynet"

-- 核心工具类库
local rpc = require "rpc"
local redis = require "redis"
local db = require "db"

skynet.start(function()
    skynet.error("============================================")
    skynet.error("Skynet Server Starting (Test Mode)...")
    skynet.error("============================================")

    -- ==================== 初始化核心模块 ====================

    -- 1. 初始化 RPC/Protobuf
    skynet.error("[1/2] Initializing RPC module...")
    if rpc.init() then
        skynet.error("[RPC] Initialized, method:", rpc.get_method())
    else
        skynet.error("[RPC] WARNING: Initialization failed!")
    end

    -- 2. 初始化 Redis
    skynet.error("[2/2] Initializing Redis module...")
    if redis.init() then
        local info = redis.info()
        skynet.error("[Redis] Initialized, mode:", info.using_redis and "Redis" or "Fallback")
    else
        skynet.error("[Redis] WARNING: Initialization failed!")
    end

    skynet.error("")

    -- ==================== 启动服务 ====================

    -- 启动 C++ 双向网关（统一处理 Skynet ↔ C++ 通信）
    -- 注意：必须先启动 gateway，db 模块才能工作
    local cpp_gateway = skynet.uniqueservice("gateway/cpp_gateway")
    skynet.error("[Service] C++ Gateway started:", skynet.address(cpp_gateway))

    -- 初始化数据库模块（gateway 已就绪）
    skynet.error("[DB] Initializing Database module...")
    if db.init() then
        local info = db.info()
        skynet.error("[DB] Initialized, gateway:", info.gateway_available and "Connected" or "Not available")
    else
        skynet.error("[DB] WARNING: Initialization failed!")
    end

    -- 启动逻辑服务（简化版，用于测试数据联通）
    local logic_service = skynet.uniqueservice("logic/logic_service")
    skynet.error("[Service] Logic Service started:", skynet.address(logic_service))

    skynet.error("============================================")
    skynet.error("Skynet Server Ready (Test Mode)")
    skynet.error("  - Gateway: C++ bidirectional communication")
    skynet.error("  - Logic: Data connectivity testing")
    skynet.error("============================================")

    -- 退出启动服务（不退出整个 Skynet）
    skynet.exit()
end)
