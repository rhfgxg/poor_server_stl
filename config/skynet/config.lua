-- Skynet 主配置文件
-- 项目根目录（注意：Skynet 从 skynet_src/skynet/ 目录启动）
root = "../../"

-- 线程数
thread = 8

-- 日志文件
logger = nil  -- nil 表示输出到标准输出

-- 守护进程
-- daemon = "./skynet.pid"  -- 取消注释以启用守护进程模式

-- Harbor 配置（单节点模式设为 0）
harbor = 0

-- 启动服务
start = "main"  -- 启动 service/main.lua

-- 启动参数
bootstrap = "snlua bootstrap"  -- 使用 snlua 引导服务

-- Lua 服务脚本路径（相对于 Skynet 运行目录 skynet_src/skynet/）
luaservice = root.."skynet_src/service/?.lua;"..
             "./service/?.lua"

-- Lua loader（相对于 Skynet 运行目录）
lualoader = "./lualib/loader.lua"

-- Lua 库路径（相对于 Skynet 运行目录）
lua_path = root.."skynet_src/lualib/?.lua;"..
           root.."config/cpp/cfg_server/?.lua;"..
           root.."config/skynet/?.lua;"..
           "./lualib/?.lua;"..
           "./lualib/?/init.lua"

-- C 服务路径（相对于 Skynet 运行目录，最关键！）
cpath = "./cservice/?.so"

-- Lua C 库路径（相对于 Skynet 运行目录）
lua_cpath = "./luaclib/?.so"

-- Snax 服务路径
snax = root.."skynet_src/service/?.lua;"..
       "./service/?.lua"

-- 调试端口（开发时使用，生产环境应关闭）
-- debug_console_port = 8000

-- C++ 网关配置
cpp_gateway_host = "0.0.0.0"
cpp_gateway_port = 8888

-- 配置文件路径（供 Lua 服务使用）
config_path = root.."config/"
cpp_config_path = root.."config/cpp/cfg_server/"

-- 注意：数据库和 Redis 配置应该在业务服务中配置
-- 可以通过 require "cfg_db" 或 require "cfg_redis" 加载配置
