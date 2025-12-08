-- Skynet 主启动脚本
local skynet = require "skynet"

skynet.start(function()
    skynet.error("============================================")
    skynet.error("Skynet Logic Server Starting...")
    skynet.error("============================================")
    
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
    
    -- 启动游戏逻辑管理器（炉石传说）
    local hearthstone_mgr = skynet.uniqueservice("logic/hearthstone_manager")
    skynet.error("Hearthstone Manager service started:", skynet.address(hearthstone_mgr))
    
    skynet.error("============================================")
    skynet.error("Skynet Logic Server Started Successfully")
    skynet.error("============================================")
    
    -- 退出启动服务（不退出整个 Skynet）
    skynet.exit()
end)
