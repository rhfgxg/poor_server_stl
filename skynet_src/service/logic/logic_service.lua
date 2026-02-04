--[[
    Logic Service (简化测试版)
    
    职责：
    1. 处理客户端通过 C++ Gateway 转发的请求
    2. 测试 Skynet ↔ C++ ↔ Database 数据联通
    3. 验证 Protobuf 消息编解码
    
    通信流程：
    Client → C++ Gateway → Skynet cpp_gateway → logic_service
                                    ↓
                              C++ DB Server
]]

local skynet = require "skynet"
local rpc = require "rpc"
local db = require "db"

local CMD = {}

-- ==================== 测试命令 ====================

-- 测试数据库读取
function CMD.test_db_read(player_id)
    skynet.error("[Logic] Testing DB read for player:", player_id)
    
    local result, err = db.read("game_db", "players", {
        player_id = player_id
    })
    
    if result then
        skynet.error("[Logic] DB read success, results:", #result)
        return { success = true, data = result }
    else
        skynet.error("[Logic] DB read failed:", err)
        return { success = false, error = err }
    end
end

-- 测试数据库写入
function CMD.test_db_write(player_id, data)
    skynet.error("[Logic] Testing DB write for player:", player_id)
    
    local ok, err = db.update("game_db", "players", 
        { player_id = player_id },
        data or { last_login = os.time() }
    )
    
    if ok then
        skynet.error("[Logic] DB write success")
        return { success = true }
    else
        skynet.error("[Logic] DB write failed:", err)
        return { success = false, error = err }
    end
end

-- 测试 Echo（用于验证通信）
function CMD.echo(message)
    skynet.error("[Logic] Echo:", message)
    return {
        success = true,
        message = message,
        server_time = os.time(),
        server = "skynet_logic"
    }
end

-- 获取服务状态
function CMD.status()
    return {
        success = true,
        service = "logic_service",
        status = "running",
        uptime = skynet.time(),
    }
end

-- ==================== 会话处理（来自 cpp_gateway 的请求）====================

-- 处理进入游戏请求
function CMD.enter_game(player_id, token)
    skynet.error("[Logic] Player entering game:", player_id)
    
    -- 1. 验证 token（实际项目中应该验证）
    -- local valid = verify_token(player_id, token)
    
    -- 2. 从数据库加载玩家数据
    local player_data, err = db.read("game_db", "players", {
        player_id = player_id
    })
    
    if not player_data or #player_data == 0 then
        skynet.error("[Logic] Player not found, creating new player")
        -- 创建新玩家
        local ok, create_err = db.create("game_db", "players", {
            player_id = player_id,
            nickname = "Player_" .. player_id,
            level = 1,
            exp = 0,
            gold = 100,
            create_time = os.time(),
            last_login = os.time(),
        })
        
        if not ok then
            return {
                success = false,
                message = "Failed to create player: " .. (create_err or "unknown")
            }
        end
        
        player_data = {{
            player_id = player_id,
            nickname = "Player_" .. player_id,
            level = 1,
            exp = 0,
            gold = 100,
        }}
    else
        -- 更新最后登录时间
        db.update("game_db", "players", 
            { player_id = player_id },
            { last_login = os.time() }
        )
    end
    
    return {
        success = true,
        message = "Welcome!",
        player_data = player_data[1],
        server_time = os.time(),
    }
end

-- 处理离开游戏请求
function CMD.leave_game(player_id, reason)
    skynet.error("[Logic] Player leaving game:", player_id, "reason:", reason)
    
    -- 保存玩家数据
    db.update("game_db", "players",
        { player_id = player_id },
        { last_logout = os.time() }
    )
    
    return {
        success = true,
        message = "Goodbye!"
    }
end

-- 处理心跳
function CMD.heartbeat(player_id, client_time)
    return {
        success = true,
        server_time = os.time(),
        latency = os.time() - (client_time or os.time())
    }
end

-- ==================== 服务入口 ====================

skynet.start(function()
    skynet.error("[Logic Service] Starting...")
    
    skynet.dispatch("lua", function(session, source, cmd, ...)
        local f = CMD[cmd]
        if f then
            skynet.ret(skynet.pack(f(...)))
        else
            skynet.error("[Logic] Unknown command:", cmd)
            skynet.ret(skynet.pack({ success = false, error = "Unknown command: " .. cmd }))
        end
    end)
    
    skynet.error("[Logic Service] Ready")
    skynet.error("  Commands: echo, status, test_db_read, test_db_write")
    skynet.error("  Session: enter_game, leave_game, heartbeat")
end)
