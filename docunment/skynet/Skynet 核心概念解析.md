# Skynet 核心概念解析

本文档深入解析Skynet框架的核心架构、设计理念和关键概念。

## 📋 目录

- [Skynet简介](#skynet简介)
- [Actor模型详解](#actor模型详解)
- [服务架构设计](#服务架构设计)
- [协程与线程](#协程与线程)
- [消息传递机制](#消息传递机制)
- [分布式架构](#分布式架构)
- [调度与性能](#调度与性能)
- [内存管理](#内存管理)
- [错误处理](#错误处理)

## 🌟 Skynet简介

### 框架定位

Skynet是一个为在线游戏服务器特别设计的轻量级框架：

- **设计目标**：高并发、低延迟、易维护
- **技术栈**：C语言核心 + Lua脚本
- **架构模式**：Actor模型 + 消息传递
- **适用场景**：MMORPG、实时策略游戏、社交游戏

### 核心特性

1. **轻量级设计**
   - 核心代码不到2万行C代码
   - 内存占用极小
   - 启动速度快

2. **高并发支持**
   - 数千个服务同时运行
   - 单机支持数万并发连接
   - 线性扩展能力

3. **开发友好**
   - Lua脚本热更新
   - 简洁的API设计
   - 丰富的调试工具

### 设计哲学

```
简单性 > 复杂性
明确性 > 隐含性
实用性 > 理论性
```

## 🎭 Actor模型详解

### 什么是Actor模型

Actor模型是一种并发计算的数学模型，每个Actor都是一个独立的计算单元：

```
Actor = 计算单元 + 私有状态 + 消息队列
```

### Skynet中的Actor

在Skynet中，每个服务(Service)就是一个Actor：

```lua
-- 每个服务都有自己的：
-- 1. 独立的Lua虚拟机状态
-- 2. 私有的数据空间
-- 3. 消息处理队列
-- 4. 唯一的地址标识

local skynet = require "skynet"
local actor_state = {}  -- 私有状态

skynet.start(function()
    -- Actor初始化
    actor_state.data = {}
    
    -- 消息处理器
    skynet.dispatch("lua", function(session, address, cmd, ...)
        -- 处理消息，修改私有状态
        if cmd == "set" then
            local key, value = ...
            actor_state.data[key] = value
        end
    end)
end)
```

### Actor模型的优势

1. **隔离性**
   - 每个Actor有独立的状态空间
   - 避免共享状态的并发问题
   - 故障隔离，一个Actor崩溃不影响其他

2. **可扩展性**
   - Actor之间通过消息通信
   - 可以分布在不同机器上
   - 支持动态创建和销毁

3. **容错性**
   - "Let it crash"理念
   - 通过监督者模式恢复
   - 系统整体稳定性高

### Actor生命周期

```lua
-- 1. 创建阶段
local service = skynet.newservice("my_service", param1, param2)

-- 2. 运行阶段（使用call或send调用服务接口）
skynet.call(service, "lua", "work", data)

-- 3. 销毁阶段
skynet.kill(service)
```

## 🏗️ 服务架构设计

### 服务类型对比

| 特性 | newservice | uniqueservice |
|------|------------|---------------|
| 实例数量 | 多个 | 单个 |
| 创建方式 | 每次调用都创建新实例 | 全局唯一实例 |
| 用途 | 业务逻辑处理 | 全局管理器 |
| 生命周期 | 手动管理 | 自动管理 |
| 负载均衡 | 支持 | 不适用 |

### 服务设计模式

#### 1. 管理器模式

```lua
-- 玩家管理器 (uniqueservice)
local players = {}

skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "create_player" then
        local player_id = ...
        local agent = skynet.newservice("player_agent", player_id)
        players[player_id] = agent
        skynet.ret(skynet.pack(agent))
    end
end)
```

#### 2. 代理模式

```lua
-- 玩家代理 (newservice)
local player_id = ...
local player_data = {}

skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "get_level" then
        skynet.ret(skynet.pack(player_data.level))
    elseif cmd == "add_exp" then
        local exp = ...
        player_data.exp = player_data.exp + exp
        check_level_up()
    end
end)
```

#### 3. 工厂模式

```lua
-- 服务工厂
local function create_worker_pool(size)
    local workers = {}
    for i = 1, size do
        workers[i] = skynet.newservice("worker")
    end
    return workers
end

-- 负载均衡分发
local current = 1
local function get_worker()
    current = current % #workers + 1
    return workers[current]
end
```

### 服务间依赖关系

```
主服务 (main)
├── 数据库服务 (uniqueservice)
├── 玩家管理器 (uniqueservice)
│   └── 玩家代理们 (newservice)
├── 聊天服务 (uniqueservice)
└── 网关服务 (uniqueservice)
    └── 连接处理器们 (newservice)
```

## ⚡ 协程与线程

### 线程模型解析

Skynet采用"少量工作线程 + 大量协程"的混合模型：

```
┌─────────────────────────────────────┐
│            Skynet进程                │
├─────────────────────────────────────┤
│ 工作线程1 │ 工作线程2 │ ... │ 工作线程N │
├─────────────────────────────────────┤
│ 协程池 (数千个协程)                    │
│ 每个服务 = 一个协程                    │
└─────────────────────────────────────┘
```

### 线程与协程创建
```
1. 服务器启动时，根据配置文件，创建固定数量的线程
2. 然后 skynet 根据服务数量创建协程：（一个服务创建一个协程）
3. 只有在处理异步消息时，才会使用fork创建副本协程（会有性能损耗，建议只在必要时使用）
    fork创建的协程为临时协程，无法被调用，任务执行完后销毁
```


### 线程职责分工

1. **主线程**
   - 初始化系统
   - 分发消息到工作线程
   - 处理定时器

2. **工作线程**
   - 执行服务协程
   - 处理网络I/O
   - 执行计算任务

3. **专用线程**
   - 网络线程：处理socket事件
   - 定时器线程：管理定时任务

### 协程调度机制

```lua
-- 协程的创建和调度
skynet.start(function()
    -- 这里运行在协程中
    
    -- 非阻塞操作
    skynet.fork(function()
        -- 新的协程
        local result = skynet.call(other_service, "lua", "work")
        process_result(result)
    end)
    
    -- 主协程继续执行
    skynet.dispatch("lua", message_handler)
end)
```

### 协程切换时机

协程会在以下情况下让出执行权：

1. **主动让出**
   ```lua
   skynet.sleep(100)  -- 休眠100个时间单位（0.01秒），在sleep时立即让出
   skynet.yield()     -- 主动让出CPU
   ```

2. **等待IO**
    网络，文件，数据库io等耗时较长的任务
   ```lua
   local result = skynet.call(service, "lua", "cmd")  -- 等待回应
   ```

3. **时间片用完**
    需要运行长时间的任务，会在cpu分配时间片用完后，被强制暂停，让出cpu
   ```lua
   -- 长时间计算会被强制切换
   for i = 1, 1000000 do
       heavy_computation()
   end
   ```

### 并发安全机制

#### 1. 消息隔离

```lua
-- 错误的做法 - 共享全局状态
global_data = {}

-- 正确的做法 - 消息传递
skynet.send(service, "lua", "update_data", data)
```

#### 2. 原子操作

```lua
-- 每个消息处理都是原子的
skynet.dispatch("lua", function(session, address, cmd, ...)
    -- 这里的代码不会被中断
    -- 直到函数返回才会处理下一个消息
    update_critical_data(...)
end)
```

#### 3. 无锁设计

Skynet通过以下方式避免锁的使用：

- 每个服务独立运行
- 消息队列FIFO处理
- 状态不共享

## 📨 消息传递机制

### 消息类型

#### 1. Lua消息

```lua
-- 发送
skynet.send(target, "lua", "command", param1, param2)
skynet.call(target, "lua", "query", data)

-- 接收
skynet.dispatch("lua", function(session, address, cmd, ...)
    -- 处理消息
end)
```

#### 2. Raw消息

```lua
-- 发送原始数据
skynet.send(target, "response", raw_data)

-- 接收原始数据
skynet.dispatch("response", function(session, address, msg, sz)
    -- msg是C指针，sz是大小
end)
```

#### 3. 文本消息

```lua
-- 发送文本
skynet.send(target, "text", "hello world")

-- 接收文本
skynet.dispatch("text", function(session, address, msg)
    print("Received:", msg)
end)
```

### 消息队列机制

每个服务都有独立的消息队列：

```
服务A                    服务B
┌─────────────┐         ┌─────────────┐
│   消息队列   │ ──────→ │   消息队列   │
│ ┌─────────┐ │         │ ┌─────────┐ │
│ │ 消息1   │ │         │ │ 消息3   │ │
│ ├─────────┤ │         │ ├─────────┤ │
│ │ 消息2   │ │         │ │ 消息4   │ │
│ └─────────┘ │         │ └─────────┘ │
└─────────────┘         └─────────────┘
```

### 消息路由

#### 1. 本地路由

```lua
-- 同一节点内的服务通信
local service = skynet.localname(".launcher")
skynet.call(service, "lua", "command")
```

#### 2. 远程路由

```lua
-- 跨节点通信
local remote_service = skynet.remotename("slave_node", "player_manager")
skynet.call(remote_service, "lua", "get_player", player_id)
```

### 消息序列化

Skynet自动处理Lua数据的序列化：

```lua
-- 复杂数据结构
local data = {
    player_id = 1001,
    items = {
        {id = 1, count = 10},
        {id = 2, count = 5}
    },
    timestamp = os.time()
}

-- 自动序列化和反序列化
skynet.call(service, "lua", "update_player", data)
```

## 🌐 分布式架构

### 节点架构

Skynet支持多节点分布式部署：

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   主节点     │    │   从节点1    │    │   从节点2    │
│  (master)   │    │  (slave1)   │    │  (slave2)   │
├─────────────┤    ├─────────────┤    ├─────────────┤
│ 全局管理器   │    │ 玩家代理群   │    │ 副本服务群   │
│ 登录服务    │    │ 聊天服务    │    │ 战斗服务    │
│ 网关服务    │    │ 邮件服务    │    │ 活动服务    │
└─────────────┘    └─────────────┘    └─────────────┘
       │                   │                   │
       └───────────────────┼───────────────────┘
                          │
                  ┌─────────────┐
                  │  集群通信    │
                  │   (TCP)     │
                  └─────────────┘
```

### skynet的分布式原理：
相比于传统分布式（每个服务独享一个main函数）skynet所有服务使用同一个main
即在 main 函数中，根据节点配置文件来决定具体启动那个服务
```lua
function main()
    local node = skynet.getenv("node_type")
    if node == "登录"
        -- 创建登录服务
    elseif node == "网关"
        -- 创建网关服务
    end
end
```

具体操作：
```lua
所有配置文件中：
添加下面内容，标记当前配置文件的类型
aaa = login -- 使用自定义变量来记录文件类型

在命令行启动 skynet时，通过参数来指定实际使用的node配置文件（只能指定一个配置文件）
skynet etc/config.node

在mian函数中 通过 
local node = skynet.getenv("aaa") -- 使用 变量名获取，传入node_type而不是login
-- 获取到 对应的类型 node == login

if node == "login"
    -- 判断获取到的值，然后创建服务
elseif node == "网关"
    -- 创建网关服务
end
```

#### 如何发现其他服务器
skynet 没有中心服务器这种注册服务器
所以需要在单个服务器的节点配置文件中，将所需的服务器地址全部写入
```lua
# etc/config.game

node_type = login

login_ip = 192.168.1.10     -- 建议将本节点的地址也写明，方便管理
login_port = 8001
gate_ip = 192.168.1.11
gate_port = 8002
```

在使用时使用 getenv api来寻找对应的地址
```lua
local address = skynet.getenv("login_ip")   -- 获取地址
skynet.call(address, "lua", cmd. 参数列表)  -- 调用
```

### 集群配置

#### 1. 主节点配置

```lua
-- master_config.lua
node_type = login   -- 节点类型

cluster = { -- 主节点和从节点的地址列表，变量名可以自定义
    master = "127.0.0.1:7001",
    slave1 = "127.0.0.1:7002", 
    slave2 = "127.0.0.1:7003"
}
```

#### 2. 从节点配置

```lua
-- slave_config.lua
node_type = login   -- 节点类型

standalone = "127.0.0.1:7002"   -- 单独写明从节点地址
cluster = {
    master = "127.0.0.1:7001",
    slave1 = "127.0.0.1:7002",
    slave2 = "127.0.0.1:7003"
}
```

### 跨节点通信

#### 1. 服务发现

```lua
-- 获取远程服务
local cluster = require "skynet.cluster"
local remote_service = cluster.query("slave1", "player_manager")
```

#### 2. 远程调用

```lua
local cluster = require "skynet.cluster"

-- 调用远程服务
local result = cluster.call("slave1", "player_manager", "get_player", 1001)

-- 发送消息到远程服务
cluster.send("slave2", "battle_service", "start_battle", battle_id)
```

#### 3. 代理模式

```lua
-- 本地代理远程服务
local cluster = require "skynet.cluster"

skynet.start(function()
    skynet.dispatch("lua", function(session, address, cmd, ...)
        -- 转发到远程节点
        local result = cluster.call("slave1", "real_service", cmd, ...)
        skynet.ret(skynet.pack(result))
    end)
end)
```

### 负载均衡

#### 1. 哈希分片

```lua
local function get_player_node(player_id)
    local nodes = {"slave1", "slave2", "slave3"}
    local index = (player_id % #nodes) + 1
    return nodes[index]
end

-- 根据玩家ID路由到不同节点
local node = get_player_node(player_id)
local result = cluster.call(node, "player_service", "get_player", player_id)
```

#### 2. 动态负载均衡

```lua
local node_loads = {
    slave1 = 100,
    slave2 = 80, 
    slave3 = 120
}

local function get_least_loaded_node()
    local min_load = math.huge
    local best_node = nil
    
    for node, load in pairs(node_loads) do
        if load < min_load then
            min_load = load
            best_node = node
        end
    end
    
    return best_node
end
```

## ⚙️ 调度与性能

### 调度策略

#### 1. 消息优先级

```lua
-- 高优先级消息
skynet.send(service, "priority", high_priority_data)

-- 普通消息
skynet.send(service, "lua", normal_data)
```

#### 2. 工作线程分配

```lua
-- 配置文件中设置
thread = 4  -- 4个工作线程

-- CPU密集型任务可以增加线程数
-- IO密集型任务线程数可以较少
```

#### 3. 协程池管理

```lua
-- 协程复用避免频繁创建销毁
local coroutine_pool = {}

local function get_coroutine()
    local co = table.remove(coroutine_pool)
    if not co then
        co = coroutine.create(work_function)
    end
    return co
end

local function return_coroutine(co)
    table.insert(coroutine_pool, co)
end
```

### 性能优化技巧

#### 1. 消息批处理

```lua
-- 避免频繁小消息
local batch = {}
for i = 1, 100 do
    table.insert(batch, data[i])
end
skynet.call(service, "lua", "batch_process", batch)
```

#### 2. 异步处理

```lua
-- 耗时操作异步化
skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "heavy_work" then
        local response = skynet.response()
        
        skynet.fork(function()
            local result = do_heavy_computation(...)
            response(true, result)
        end)
    end
end)
```

#### 3. 缓存策略

```lua
-- 本地缓存热点数据
local cache = {}
local cache_time = {}

local function get_data(key)
    local now = skynet.time()
    if cache[key] and now - cache_time[key] < 300 then
        return cache[key]  -- 5分钟缓存
    end
    
    local data = load_from_database(key)
    cache[key] = data
    cache_time[key] = now
    return data
end
```

### 内存管理

#### 1. 垃圾回收

```lua
-- 定期触发GC
skynet.timeout(3000, function()  -- 30秒
    collectgarbage("collect")
    skynet.timeout(3000, gc_timer)
end)
```

#### 2. 内存监控

```lua
-- 监控内存使用
local function memory_monitor()
    local mem = collectgarbage("count")
    if mem > 100 * 1024 then  -- 100MB
        skynet.error("High memory usage:", mem, "KB")
    end
end
```

#### 3. 对象池

```lua
-- 重用对象减少GC压力
local object_pool = {}

local function get_object()
    local obj = table.remove(object_pool)
    if not obj then
        obj = {data = {}}
    else
        -- 重置对象状态
        for k in pairs(obj.data) do
            obj.data[k] = nil
        end
    end
    return obj
end

local function return_object(obj)
    table.insert(object_pool, obj)
end
```

## 🚨 错误处理

### 错误类型

#### 1. 服务崩溃

```lua
-- 监控服务状态
local function monitor_service(service)
    skynet.fork(function()
        while true do
            local ok = pcall(skynet.call, service, "lua", "ping")
            if not ok then
                skynet.error("Service crashed, restarting...")
                service = skynet.newservice("my_service")
            end
            skynet.sleep(1000)  -- 10秒检查一次
        end
    end)
end
```

#### 2. 消息处理错误

```lua
skynet.dispatch("lua", function(session, address, cmd, ...)
    local ok, err = pcall(function()
        if cmd == "work" then
            return process_work(...)
        end
    end)
    
    if ok then
        skynet.ret(skynet.pack(err))  -- err是返回值
    else
        skynet.error("Message processing error:", err)
        skynet.ret(skynet.pack(nil, err))  -- 返回错误
    end
end)
```

#### 3. 网络错误

```lua
local function safe_remote_call(node, service, cmd, ...)
    local ok, result = pcall(cluster.call, node, service, cmd, ...)
    if not ok then
        skynet.error("Remote call failed:", result)
        return nil, result
    end
    return result
end
```

### 容错机制

#### 1. 重试机制

```lua
local function retry_call(service, cmd, data, max_retry)
    max_retry = max_retry or 3
    
    for i = 1, max_retry do
        local ok, result = pcall(skynet.call, service, "lua", cmd, data)
        if ok then
            return result
        end
        
        skynet.error("Retry", i, "failed:", result)
        if i < max_retry then
            skynet.sleep(100 * i)  -- 递增延迟
        end
    end
    
    error("Max retry exceeded")
end
```

#### 2. 熔断器模式

```lua
local circuit_breaker = {
    failure_count = 0,
    failure_threshold = 5,
    timeout_period = 300,  -- 5分钟
    last_failure_time = 0,
    state = "CLOSED"  -- CLOSED, OPEN, HALF_OPEN
}

local function call_with_circuit_breaker(service, cmd, data)
    local now = skynet.time()
    
    if circuit_breaker.state == "OPEN" then
        if now - circuit_breaker.last_failure_time > circuit_breaker.timeout_period then
            circuit_breaker.state = "HALF_OPEN"
        else
            error("Circuit breaker is OPEN")
        end
    end
    
    local ok, result = pcall(skynet.call, service, "lua", cmd, data)
    
    if ok then
        circuit_breaker.failure_count = 0
        circuit_breaker.state = "CLOSED"
        return result
    else
        circuit_breaker.failure_count = circuit_breaker.failure_count + 1
        circuit_breaker.last_failure_time = now
        
        if circuit_breaker.failure_count >= circuit_breaker.failure_threshold then
            circuit_breaker.state = "OPEN"
        end
        
        error(result)
    end
end
```

#### 3. 优雅降级

```lua
local function get_player_data(player_id)
    -- 尝试从缓存获取
    local cache_data = get_from_cache(player_id)
    if cache_data then
        return cache_data
    end
    
    -- 尝试从数据库获取
    local ok, db_data = pcall(skynet.call, db_service, "lua", "get_player", player_id)
    if ok then
        set_to_cache(player_id, db_data)
        return db_data
    end
    
    -- 降级：返回默认数据
    skynet.error("Database unavailable, using default data for player:", player_id)
    return get_default_player_data()
end
```

### 日志与监控

#### 1. 结构化日志

```lua
local function log_error(module, action, error_msg, context)
    local log_data = {
        timestamp = os.date("%Y-%m-%d %H:%M:%S"),
        level = "ERROR",
        module = module,
        action = action,
        message = error_msg,
        context = context
    }
    
    skynet.error("LOG:", table.concat({
        log_data.timestamp,
        log_data.level,
        log_data.module,
        log_data.action,
        log_data.message,
        skynet.pack(log_data.context)
    }, " | "))
end
```

#### 2. 性能监控

```lua
local function performance_monitor(func_name, func, ...)
    local start_time = skynet.time()
    local ok, result = pcall(func, ...)
    local end_time = skynet.time()
    
    local duration = end_time - start_time
    if duration > 100 then  -- 超过1秒
        skynet.error("SLOW OPERATION:", func_name, "took", duration / 100, "seconds")
    end
    
    if not ok then
        skynet.error("OPERATION FAILED:", func_name, result)
        error(result)
    end
    
    return result
end
```

---

通过深入理解这些核心概念，你将能够更好地设计和实现基于Skynet的高性能游戏服务器系统。

*更新时间：2025年8月22日*
