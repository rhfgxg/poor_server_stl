-- Skynet 主启动脚本
local skynet = require "skynet"
local proto_util = require "proto_util"

skynet.start(function()
    skynet.error("============================================")
    skynet.error("Skynet Logic Server Starting...")
    skynet.error("============================================")
    
    -- 初始化 Protobuf（第一步）
    skynet.error("\n[Protobuf] Initializing Protocol Buffers...")
    local ok, err = pcall(proto_util.load_proto)
    if not ok then
        skynet.error("[Protobuf] WARNING: Failed to load proto from file:", err)
        skynet.error("[Protobuf] Using embedded proto definitions as fallback")
        -- 尝试加载内嵌定义
        local embedded_ok, embedded_err = pcall(proto_util.load_proto_embedded)
        if not embedded_ok then
            skynet.error("[Protobuf] CRITICAL: Failed to load embedded proto:", embedded_err)
            skynet.error("[Protobuf] System may not function correctly!")
        else
            skynet.error("[Protobuf] Embedded proto loaded successfully")
        end
    else
        skynet.error("[Protobuf] Proto definitions loaded successfully")
    end
    
    -- 快速验证 Protobuf 功能（可选，生产环境可注释）
    skynet.error("[Protobuf] Running quick validation...")
    local test_data = {
        player_id = "system_check",
        token = "validate_proto",
        client_version = "1.0.0"
    }
    
    local binary, encode_err = proto_util.encode(proto_util.MessageType.MSG_ENTER_GAME, test_data)
    if binary then
        local decoded, decode_err = proto_util.decode(proto_util.MessageType.MSG_ENTER_GAME, binary)
        if decoded and decoded.player_id == test_data.player_id then
            skynet.error("[Protobuf] Validation PASSED - System ready")
        else
            skynet.error("[Protobuf] Validation FAILED - Decode error:", decode_err or "data mismatch")
        end
    else
        skynet.error("[Protobuf] Validation FAILED - Encode error:", encode_err)
    end
    
    skynet.error("")  -- 空行分隔
    
    -- 启动调试控制台（可选，开发时启用）
    -- local console_port = skynet.getenv("debug_console_port")
    -- if console_port then
    --     skynet.newservice("debug_console", console_port)
    --     skynet.error("Debug console started on port:", console_port)
    -- end
    
    -- 启动 C++ 网关服务（接收来自 C++ Logic Server 的连接）
    local cpp_gateway = skynet.uniqueservice("cpp_gateway")
    skynet.error("C++ Gateway service started:", skynet.address(cpp_gateway))
    
    -- 启动玩家管理器
    local player_mgr = skynet.uniqueservice("player_manager")
    skynet.error("Player Manager service started:", skynet.address(player_mgr))
    
    -- 启动成就服务
    local achievement_srv = skynet.uniqueservice("logic/achievement_service")
    skynet.error("Achievement service started:", skynet.address(achievement_srv))
    
    -- 启动游戏逻辑管理器（炉石传说）
    local hearthstone_mgr = skynet.uniqueservice("logic/hearthstone_manager")
    skynet.error("Hearthstone Manager service started:", skynet.address(hearthstone_mgr))
    
    skynet.error("============================================")
    skynet.error("Skynet Logic Server Started Successfully")
    skynet.error("All services online and ready")
    skynet.error("============================================")
    
    -- 退出启动服务（不退出整个 Skynet）
    skynet.exit()
end)
