-- Redis Bridge 测试配置

root = "../../"
thread = 2
logger = nil
harbor = 0
start = "test/redis_bridge_test"  -- 启动 Redis Bridge 测试服务
bootstrap = "snlua bootstrap"

-- Lua 服务路径
luaservice = root.."skynet_src/service/?.lua;./service/?.lua"
lualoader = "./lualib/loader.lua"

-- Lua 模块路径
lua_path = root.."skynet_src/lualib/?.lua;./lualib/?.lua;./lualib/?/init.lua"

-- C 服务路径
cpath = "./cservice/?.so"
lua_cpath = "./luaclib/?.so"

-- Snax 服务路径
snax = root.."skynet_src/service/?.lua;./service/?.lua"
