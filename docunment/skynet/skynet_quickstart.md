# Skynet 快速入门指南

本指南将帮助您快速上手Skynet框架，从环境搭建到项目开发。

## 📋 目录

- [环境准备](#环境准备)
- [项目搭建](#项目搭建)
- [第一个服务](#第一个服务)
- [服务间通信](#服务间通信)
- [常用模式](#常用模式)
- [项目结构](#项目结构)
- [部署运行](#部署运行)

## 🚀 环境准备

### 系统要求
- Linux / macOS / Windows (WSL)
- GCC 编译器
- Make 工具
- Git

### 安装Skynet

```bash
# 克隆源码
git clone https://github.com/cloudwu/skynet.git
cd skynet

# 编译
make linux    # Linux系统
# 或
make macosx   # macOS系统
```

### 验证安装

```bash
./skynet examples/config    # 运行示例
```

如果看到输出日志，说明安装成功。

## 🏗️ 项目搭建

### 创建项目目录

```bash
mkdir my_game_server
cd my_game_server
```

### 项目结构

```
my_game_server/
├── config.lua          # 配置文件
├── main.lua            # 主服务
├── service/            # 业务服务
├── lualib/             # 自定义库
└── skynet/             # Skynet框架(软链接)
```

### 基础配置文件

创建 `config.lua`：

```lua
thread = 4                      -- 工作线程数
start = "main"                  -- 启动服务
bootstrap = "snlua bootstrap"   -- 引导服务
logger = nil                    -- 日志配置
logpath = "."                   -- 日志路径

-- 路径配置
root = "./"
luaservice = root.."service/?.lua;"..root.."skynet/service/?.lua"
lualoader = root.."skynet/lualib/loader.lua"
lua_path = root.."lualib/?.lua;"..root.."skynet/lualib/?.lua"
lua_cpath = root.."luaclib/?.so;"..root.."skynet/luaclib/?.so"
```

## 📝 第一个服务

### 创建主服务

创建 `main.lua`：

```lua
local skynet = require "skynet"

skynet.start(function()
    skynet.error("=== Game Server Starting ===")
    
    -- 创建一个工作服务
    local worker = skynet.newservice("worker")
    
    -- 测试服务通信
    local result = skynet.call(worker, "lua", "hello", "Skynet")
    skynet.error("Worker response:", result)
    
    -- 创建数据库服务
    skynet.uniqueservice("database")
    
    skynet.error("=== Game Server Started ===")
    
    -- 保持主服务运行
    skynet.dispatch("lua", function() end)
end)
```

### 创建工作服务

创建 `service/worker.lua`：

```lua
local skynet = require "skynet"

skynet.start(function()
    skynet.error("Worker service started")
    

    skynet.dispatch("lua", function(session, address, cmd, ...)
        if cmd == "hello" then
            local name = ...
            local response = "Hello, " .. name .. "!"
            skynet.error("Processing hello request for:", name)
            skynet.ret(skynet.pack(response))   -- 回复给 call 的返回值（必须）
        else
            skynet.error("Unknown command:", cmd)
            skynet.ret(skynet.pack("error"))
        end
    end)
end)
```

### 运行项目

```bash
# 创建软链接到skynet
ln -s /path/to/skynet ./skynet

# 运行项目
./skynet/skynet config.lua
```

## 📡 服务间通信

### 基本通信模式

#### 1. 单向消息 (send)

发送消息，不等待回复：

```lua
-- 发送方

-- target_service: 被调用方唯一地址，使用 queryservice 寻找已存在的服务地址
-- lua: 协议类型
-- notify: 传递的 cmd
-- data: 传递的参数
skynet.send(target_service, "lua", "notify", data)

-- 接收方

-- 每个服务的每种协议都需要创建唯一的 dispatch 用来处理 send 和 call 调用
-- lua: 协议类型
-- session: 消息id：call对应的id递增，send=0（判断session > 0 来判断是否需要回复）
-- address: 调用方地址（可以使用 call(address, ...) 来调用被调用方）
-- cmd 对应调用函数的 第三个参数
-- ... 调用方的第四个及以后的参数
skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "notify" then
        local data = ...
        -- 处理通知，无需回复
        process_notification(data)
    end
end)
```

#### 2. 双向消息 (call)

发送消息并等待回复：
阻塞调用方，直到回复

```lua
-- 调用方
local result = skynet.call(target_service, "lua", "query", params)

-- 接收方
skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "query" then
        local params = ...
        local result = process_query(params)
        skynet.ret(skynet.pack(result))  -- 必须回复
    end
end)
```

#### 3. 异步回复

对于耗时操作，可以异步回复：

```lua
skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "async_task" then
        local response = skynet.response()
        
        skynet.fork(function()
            -- 异步处理耗时任务
            local result = do_heavy_work(...)
            response(true, result)
        end)
    end
end)
```

### 消息数据处理

```lua
-- 发送复杂数据
local data = {
    player_id = 1001,
    items = {1, 2, 3},
    info = "test"
}
skynet.send(service, "lua", "update", data)

-- 接收并解析
skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "update" then
        local data = ...
        local player_id = data.player_id
        local items = data.items
        -- 处理数据...
    end
end)
```

## 🔧 常用模式

### 1. 数据库服务模式

创建 `service/database.lua`：

```lua
local skynet = require "skynet"

local db_data = {}  -- 模拟数据库

skynet.start(function()
    skynet.error("Database service started")
    
    skynet.dispatch("lua", function(session, address, cmd, ...)
        if cmd == "save" then
            local key, value = ...
            db_data[key] = value
            skynet.ret(skynet.pack("ok"))
            
        elseif cmd == "load" then
            local key = ...
            local value = db_data[key]
            skynet.ret(skynet.pack(value))
            
        elseif cmd == "delete" then
            local key = ...
            db_data[key] = nil
            skynet.ret(skynet.pack("ok"))
        end
    end)
end)
```

### 2. 玩家代理模式

创建 `service/player.lua`：

```lua
local skynet = require "skynet"

skynet.start(function()
    local player_id = ...
    local player_data = {
        id = player_id,
        level = 1,
        exp = 0
    }
    
    skynet.error("Player agent started for player:", player_id)
    
    skynet.dispatch("lua", function(session, address, cmd, ...)
        if cmd == "get_info" then
            skynet.ret(skynet.pack(player_data))
            
        elseif cmd == "add_exp" then
            local exp = ...
            player_data.exp = player_data.exp + exp
            
            -- 升级检查
            if player_data.exp >= 100 then
                player_data.level = player_data.level + 1
                player_data.exp = 0
                skynet.error("Player level up:", player_data.level)
            end
            
            skynet.ret(skynet.pack(player_data))
            
        elseif cmd == "save" then
            -- 保存到数据库
            local db = skynet.queryservice("database")
            skynet.call(db, "lua", "save", "player_"..player_id, player_data)
            skynet.ret(skynet.pack("ok"))
        end
    end)
end)
```

### 3. 管理器模式

创建 `service/player_manager.lua`：

```lua
local skynet = require "skynet"

local players = {}  -- 在线玩家列表

skynet.start(function()
    skynet.error("Player manager started")
    
    skynet.dispatch("lua", function(session, address, cmd, ...)
        if cmd == "login" then
            local player_id = ...
            
            if players[player_id] then
                skynet.ret(skynet.pack(nil, "already online"))
                return
            end
            
            -- 创建玩家代理
            local agent = skynet.newservice("player", player_id)
            players[player_id] = agent
            
            skynet.error("Player logged in:", player_id)
            skynet.ret(skynet.pack(agent))
            
        elseif cmd == "logout" then
            local player_id = ...
            local agent = players[player_id]
            
            if agent then
                -- 保存玩家数据
                skynet.call(agent, "lua", "save")
                -- 销毁代理
                skynet.kill(agent)
                players[player_id] = nil
                skynet.error("Player logged out:", player_id)
            end
            
            skynet.ret(skynet.pack("ok"))
            
        elseif cmd == "get_online_count" then
            local count = 0
            for _ in pairs(players) do
                count = count + 1
            end
            skynet.ret(skynet.pack(count))
        end
    end)
end)
```

## 📁 项目结构

### 推荐的目录结构

```
game_server/
├── config.lua              # 主配置
├── main.lua                # 入口服务
├── service/                # 业务服务
│   ├── player.lua          # 玩家代理
│   ├── player_manager.lua  # 玩家管理器
│   ├── database.lua        # 数据库服务
│   └── gateway.lua         # 网关服务
├── lualib/                 # 自定义库
│   ├── protocol.lua        # 协议处理
│   ├── utils.lua           # 工具函数
│   └── config.lua          # 配置数据
├── script/                 # 脚本工具
│   ├── start.sh           # 启动脚本
│   └── stop.sh            # 停止脚本
└── log/                   # 日志目录
```

### 完整的main.lua示例

```lua
local skynet = require "skynet"

skynet.start(function()
    skynet.error("=== Game Server Starting ===")
    
    -- 启动核心服务
    skynet.uniqueservice("database")
    skynet.uniqueservice("player_manager")
    
    -- 启动网关服务
    local gateway = skynet.uniqueservice("gateway")
    
    skynet.error("=== All services started ===")
    skynet.error("Server is ready for connections")
    
    -- 主服务消息处理
    skynet.dispatch("lua", function(session, address, cmd, ...)
        if cmd == "status" then
            local mgr = skynet.queryservice("player_manager")
            local count = skynet.call(mgr, "lua", "get_online_count")
            skynet.ret(skynet.pack({
                online_players = count,
                server_time = skynet.time()
            }))
        end
    end)
end)
```

### skynet 服务器启动全流程

```
1. Skynet进程启动
    ↓
2. 框架系统准备（bootstrap引导服务）
    ├── 创建工作线程池
    ├── 启动launcher（服务管理器）
    ├── 启动logger（日志系统）
    └── 初始化服务注册表
    ↓
3. 加载并执行main.lua（不是载入所有文件）
    ├── require "skynet"
    ├── 初始化main的全局变量
    └── 执行skynet.start(function() ... end)
    ↓
4. main.lua中创建其他服务
    ├── skynet.uniqueservice("database") → 加载service/database.lua
    ├── skynet.uniqueservice("player_manager") → 加载service/player_manager.lua
    └── 每个服务加载时才执行对应的lua文件
    ↓
5. 各服务注册dispatch，准备接收消息
    ↓
6. 服务器启动完成，等待消息处理
```

#### 🧪 详细流程演示

**1. 启动配置 (config.lua)**
```lua
start = "main"                  -- 指定启动脚本
bootstrap = "snlua bootstrap"   -- 指定引导程序
```

**2. 主服务 (main.lua)**
```lua
local skynet = require "skynet"

print("main.lua脚本开始执行")  -- ← 立即执行

skynet.start(function()
    print("main服务启动")  -- ← skynet.start被框架调用时执行
    
    -- 创建数据库服务（此时才加载database.lua）
    print("准备创建数据库服务")
    local db = skynet.uniqueservice("database")
    print("数据库服务创建完成，地址:", db)
    
    -- 创建玩家管理器（此时才加载player_manager.lua）
    print("准备创建玩家管理器")
    local mgr = skynet.uniqueservice("player_manager")
    print("玩家管理器创建完成，地址:", mgr)
    
    -- 测试服务通信
    print("测试数据库服务")
    skynet.call(db, "lua", "save", "test_key", "test_value")
    local value = skynet.call(db, "lua", "load", "test_key")
    print("数据库测试结果:", value)
    
    skynet.dispatch("lua", function() end)  -- 保持main服务运行
end)
```

**3. 数据库服务 (service/database.lua)**
```lua
local skynet = require "skynet"

print("database.lua脚本开始执行")  -- ← uniqueservice调用时执行

local db_data = {}  -- ← 立即执行

skynet.start(function()
    print("数据库服务启动完成")  -- ← 服务创建时执行
    
    skynet.dispatch("lua", function(session, address, cmd, ...)
        print("数据库收到命令:", cmd)
        
        if cmd == "save" then
            local key, value = ...
            db_data[key] = value
            print("保存数据:", key, "=", value)
            skynet.ret(skynet.pack("ok"))
            
        elseif cmd == "load" then
            local key = ...
            local value = db_data[key]
            print("加载数据:", key, "=", value)
            skynet.ret(skynet.pack(value))
        end
    end)
end)
```

#### 📊 实际运行输出

```bash
$ ./skynet config.lua

# 框架启动
[:01000000] LAUNCH snlua bootstrap
[:01000001] LAUNCH snlua launcher
[:01000002] LAUNCH snlua main

# main.lua执行
main.lua脚本开始执行
main服务启动
准备创建数据库服务

# database.lua加载并执行
[:01000003] LAUNCH snlua database
database.lua脚本开始执行  
数据库服务启动完成

数据库服务创建完成，地址: 16777219
准备创建玩家管理器

# player_manager.lua加载并执行
[:01000004] LAUNCH snlua player_manager
player_manager.lua脚本开始执行
玩家管理器启动完成

玩家管理器创建完成，地址: 16777220
测试数据库服务
数据库收到命令: save
保存数据: test_key = test_value
数据库收到命令: load
加载数据: test_key = test_value
数据库测试结果: test_value
```

## 🚀 部署运行

### 开发环境

```bash
# 直接运行
./skynet/skynet config.lua

# 后台运行
nohup ./skynet/skynet config.lua > server.log 2>&1 &
```

### 生产环境配置

创建 `config_prod.lua`：

```lua
thread = 8                      -- 增加线程数
daemon = "./game_server.pid"    -- 守护进程
logger = "logger"               -- 启用日志服务
logpath = "/var/log/gameserver" -- 日志路径

-- 其他配置保持一致
start = "main"
bootstrap = "snlua bootstrap"
-- ...
```

### 启动脚本

创建 `script/start.sh`：

```bash
#!/bin/bash

SERVER_DIR="/opt/game_server"
SKYNET_BIN="$SERVER_DIR/skynet/skynet"
CONFIG_FILE="$SERVER_DIR/config_prod.lua"
PID_FILE="$SERVER_DIR/game_server.pid"

cd $SERVER_DIR

# 检查是否已运行
if [ -f $PID_FILE ]; then
    PID=$(cat $PID_FILE)
    if kill -0 $PID 2>/dev/null; then
        echo "Server is already running (PID: $PID)"
        exit 1
    fi
fi

# 启动服务器
echo "Starting game server..."
$SKYNET_BIN $CONFIG_FILE

if [ $? -eq 0 ]; then
    echo "Game server started successfully"
else
    echo "Failed to start game server"
    exit 1
fi
```

### 停止脚本

创建 `script/stop.sh`：

```bash
#!/bin/bash

PID_FILE="/opt/game_server/game_server.pid"

if [ -f $PID_FILE ]; then
    PID=$(cat $PID_FILE)
    echo "Stopping game server (PID: $PID)..."
    kill $PID
    
    # 等待进程结束
    while kill -0 $PID 2>/dev/null; do
        sleep 1
    done
    
    rm -f $PID_FILE
    echo "Game server stopped"
else
    echo "Server is not running"
fi
```

### 监控脚本

创建 `script/monitor.sh`：

```bash
#!/bin/bash

PID_FILE="/opt/game_server/game_server.pid"
CONFIG_FILE="/opt/game_server/config_prod.lua"

check_server() {
    if [ -f $PID_FILE ]; then
        PID=$(cat $PID_FILE)
        if kill -0 $PID 2>/dev/null; then
            return 0  # 运行中
        fi
    fi
    return 1  # 未运行
}

if ! check_server; then
    echo "$(date): Server is down, restarting..."
    cd /opt/game_server
    ./skynet/skynet $CONFIG_FILE
fi
```

## 🎯 下一步

完成快速入门后，建议：

1. 阅读 [核心概念解析](./skynet_concepts.md) 深入理解框架
2. 参考 [API参考手册](./skynet_api_reference.md) 学习更多功能
3. 查看官方示例项目
4. 实践更复杂的游戏逻辑

---

*更新时间：2025年8月22日*
