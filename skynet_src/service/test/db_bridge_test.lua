-- DB Bridge 测试服务

local skynet = require "skynet"
local db_bridge = require "db_bridge"

skynet.start(function()
    skynet.error("====================================")
    skynet.error("DB Bridge Test Service")
    skynet.error("====================================")
    skynet.error("")
    
    -- 1. 初始化数据库
    skynet.error("[Test] Step 1: Initializing database...")
    local ok = db_bridge.init()
    if not ok then
        skynet.error("[Test] FAILED: Database initialization failed")
        return
    end
    skynet.error("[Test] SUCCESS: Database initialized")
    skynet.error("")
    
    -- 2. 获取信息
    local info = db_bridge.get_info()
    skynet.error("[Test] Database Info:")
    skynet.error("  - Connected:", info.connected)
    skynet.error("  - Using mock:", info.using_mock)
    skynet.error("")
    
    -- 3. 运行测试
    skynet.error("[Test] Step 2: Running database tests...")
    local test_ok = db_bridge.test()
    if not test_ok then
        skynet.error("[Test] FAILED: Database tests failed")
        return
    end
    skynet.error("")
    
    -- 4. 测试完整数据流程
    skynet.error("[Test] Step 3: Testing full data workflow...")
    
    local test_player_id = "workflow_test_player"
    
    -- 保存完整数据
    local player_data = {
        level = 20,
        exp = 15000,
        gold = 50000,
        achievements = {
            progress = {
                ach_flow_001 = 30,
                ach_flow_002 = 75,
                ach_flow_003 = 100
            },
            completed = {
                ach_flow_003 = true
            }
        },
        cards = {
            card_001 = {count = 2, level = 5},
            card_002 = {count = 1, level = 3}
        },
        last_login = os.time()
    }
    
    local save_ok = db_bridge.save_player_full(test_player_id, player_data)
    if not save_ok then
        skynet.error("[Test] FAILED: save_player_full")
        return
    end
    skynet.error("[Test] SUCCESS: Full data saved")
    
    -- 加载完整数据
    local loaded_data = db_bridge.load_player_full(test_player_id)
    if not loaded_data then
        skynet.error("[Test] FAILED: load_player_full")
        return
    end
    
    skynet.error("[Test] Loaded player data:")
    skynet.error("  - Player ID:", loaded_data.player_id)
    skynet.error("  - Level:", loaded_data.level)
    skynet.error("  - Exp:", loaded_data.exp)
    skynet.error("  - Gold:", loaded_data.gold)
    
    -- 计算成就数量
    local ach_count = 0
    for _ in pairs(loaded_data.achievements.progress or {}) do
        ach_count = ach_count + 1
    end
    skynet.error("  - Achievements:", ach_count, "items")
    
    -- 列出卡牌
    local card_list = {}
    for card_id, _ in pairs(loaded_data.cards or {}) do
        table.insert(card_list, card_id)
    end
    skynet.error("  - Cards:", table.concat(card_list, ", "))
    
    -- 验证数据
    if loaded_data.level ~= 20 or loaded_data.exp ~= 15000 or loaded_data.gold ~= 50000 then
        skynet.error("[Test] FAILED: Data mismatch")
        return
    end
    
    skynet.error("[Test] SUCCESS: Full data workflow completed")
    skynet.error("")
    
    -- 5. 完成
    skynet.error("====================================")
    skynet.error("✓ All database tests PASSED!")
    skynet.error("====================================")
    skynet.error("")
    skynet.error("DB Bridge is ready to use!")
    skynet.error("")
end)
