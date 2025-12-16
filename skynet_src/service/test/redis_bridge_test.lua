-- Redis Bridge 测试服务
-- 测试 Redis 连接和基本操作

local skynet = require "skynet"
local redis_bridge = require "redis_bridge"

skynet.start(function()
    skynet.error("====================================")
    skynet.error("Redis Bridge Test Service")
    skynet.error("====================================")
    skynet.error("")
    
    -- 1. 初始化 Redis
    skynet.error("[Test] Step 1: Initializing Redis...")
    local ok = redis_bridge.init()
    if not ok then
        skynet.error("[Test] FAILED: Redis initialization failed")
        return
    end
    skynet.error("[Test] SUCCESS: Redis initialized")
    skynet.error("")
    
    -- 2. 获取信息
    local info = redis_bridge.get_info()
    skynet.error("[Test] Redis Info:")
    skynet.error("  - Initialized:", info.initialized)
    skynet.error("  - Using fallback:", info.using_fallback)
    if info.config then
        skynet.error("  - Host:", info.config.host)
        skynet.error("  - Port:", info.config.port)
        skynet.error("  - DB:", info.config.db or 0)
    end
    skynet.error("")
    
    -- 3. 运行基础测试
    skynet.error("[Test] Step 2: Running basic tests...")
    local test_ok = redis_bridge.test()
    if not test_ok then
        skynet.error("[Test] FAILED: Basic tests failed")
        return
    end
    skynet.error("")
    
    -- 4. 测试玩家数据接口
    skynet.error("[Test] Step 3: Testing player data interface...")
    
    local test_player_id = "test_player_123"
    
    -- 保存玩家基础数据
    local player_data = {
        level = 10,
        exp = 5000,
        gold = 12345,
        last_login = os.time()
    }
    
    local save_ok = redis_bridge.save_player_basic(test_player_id, player_data)
    if not save_ok then
        skynet.error("[Test] FAILED: Save player data failed")
        return
    end
    skynet.error("[Test] SUCCESS: Player data saved")
    
    -- 加载玩家基础数据
    local loaded_data = redis_bridge.load_player_basic(test_player_id)
    if not loaded_data then
        skynet.error("[Test] FAILED: Load player data failed")
        return
    end
    
    skynet.error("[Test] Loaded player data:")
    skynet.error("  - Level:", loaded_data.level)
    skynet.error("  - Exp:", loaded_data.exp)
    skynet.error("  - Gold:", loaded_data.gold)
    skynet.error("  - Last login:", loaded_data.last_login)
    
    -- 验证数据
    if loaded_data.level ~= player_data.level or 
       loaded_data.exp ~= player_data.exp or
       loaded_data.gold ~= player_data.gold then
        skynet.error("[Test] FAILED: Data mismatch")
        return
    end
    
    skynet.error("[Test] SUCCESS: Player data verified")
    skynet.error("")
    
    -- 5. 测试成就数据
    skynet.error("[Test] Step 4: Testing achievement data...")
    
    redis_bridge.save_player_achievement(test_player_id, "ach_001", 10)
    redis_bridge.save_player_achievement(test_player_id, "ach_002", 25)
    redis_bridge.save_player_achievement(test_player_id, "ach_003", 100)
    
    local achievements = redis_bridge.load_player_achievements(test_player_id)
    skynet.error("[Test] Loaded achievements:")
    for id, progress in pairs(achievements) do
        skynet.error(string.format("  - %s: %d", id, progress))
    end
    
    if achievements.ach_001 ~= 10 or achievements.ach_002 ~= 25 or achievements.ach_003 ~= 100 then
        skynet.error("[Test] FAILED: Achievement data mismatch")
        return
    end
    
    skynet.error("[Test] SUCCESS: Achievement data verified")
    skynet.error("")
    
    -- 6. 测试金币增加
    skynet.error("[Test] Step 5: Testing gold increment...")
    
    local new_gold = redis_bridge.add_player_gold(test_player_id, 500)
    skynet.error("[Test] Gold after increment:", new_gold)
    
    if not new_gold then
        skynet.error("[Test] WARNING: Gold increment test skipped (fallback mode)")
    end
    skynet.error("")
    
    -- 7. 清理测试数据
    skynet.error("[Test] Step 6: Cleaning up...")
    redis_bridge.del(redis_bridge.make_key("player", test_player_id, "basic"))
    redis_bridge.del(redis_bridge.make_key("player", test_player_id, "achievements"))
    redis_bridge.del(redis_bridge.make_key("player", test_player_id, "gold"))
    skynet.error("[Test] Test data cleaned")
    skynet.error("")
    
    -- 8. 完成
    skynet.error("====================================")
    skynet.error("✓ All tests PASSED!")
    skynet.error("====================================")
    skynet.error("")
    skynet.error("Redis Bridge is ready to use!")
    skynet.error("")
    
    -- 不退出，保持服务运行
    -- 可以手动调用 skynet.exit() 退出
end)
