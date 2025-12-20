# Skynet API 参考手册

完整的Skynet框架API参考文档，按功能模块分类整理。

## 📋 目录

- [核心API](#核心api)
- [服务管理](#服务管理)
- [消息通信](#消息通信)
- [定时器](#定时器)
- [网络通信](#网络通信)
- [数据库](#数据库)
- [集群通信](#集群通信)
- [配置管理](#配置管理)
- [调试工具](#调试工具)
- [工具函数](#工具函数)

## 🔧 核心API

### skynet.start(func)

启动一个服务，所有服务的入口点。在服务创建时使用
一般包含服务消息接口注册，日志等基础功能的加载

**参数：**
- `func`: 服务启动函数

**示例：**
```lua
local skynet = require "skynet"

skynet.start(function()
    skynet.error("Service started")
    -- 服务初始化代码
end)
```

### skynet.exit()

退出当前服务。

**示例：**
```lua
skynet.exit()
```

### skynet.abort()

强制终止整个Skynet进程。

**示例：**
```lua
skynet.abort()
```

### skynet.getenv(key)

获取配置项的值。

**参数：**
- `key`: 配置项名称

**返回值：**
- 配置项的值，如果不存在返回nil

**示例：**
```lua
local thread_count = skynet.getenv("thread")
local start_service = skynet.getenv("start")
```

### skynet.setenv(key, value)

设置配置项的值。不会保存到配置文件，仅在本次启动有效

**参数：**
- `key`: 配置项名称
- `value`: 配置项的值

**示例：**
```lua
skynet.setenv("daemon", "./game.pid")
```

## 🎯 服务管理

### skynet.newservice(name, ...)

创建一个新的服务实例。

**参数：**
- `name`: 服务脚本名称
- `...`: 传递给服务的参数
    + 传入的参数，会传给服务脚本的入口（即文件最外层）
    + 使用 ... 或 {...} select('#', ...)
```lua
    -- 创建服务时
    local db = skynet.newservice("database", "127.0.0.1", 3306, "user", "pass")

    -- service/database.lua
    local host, port, user, pass = ...
    local args = {...}
    local aaa = select('#', ...)
    skynet.start(function()
        -- 这里可以直接用 host, port, user, pass
    end)
```

**返回值：**
- 服务地址

**示例：**
```lua
local worker = skynet.newservice("worker", "param1", "param2")
local player_agent = skynet.newservice("player", player_id)
```

### skynet.uniqueservice(name, ...)

创建或获取一个全局唯一的服务。

**参数：**
- `name`: 服务脚本名称
- `...`: 传递给服务的参数

**返回值：**
- 服务地址

**示例：**
```lua
local db = skynet.uniqueservice("database")
local player_mgr = skynet.uniqueservice("player_manager")
```

### skynet.queryservice(name)

查询已存在的全局唯一服务。

**参数：**
- `name`: 服务名称

**返回值：**
- 服务地址，如果不存在则抛出错误

**示例：**
```lua
local db = skynet.queryservice("database")
```

### skynet.kill(service)

销毁一个服务。

**参数：**
- `service`: 服务地址

**示例：**
```lua
skynet.kill(worker_service)
```

### skynet.self()

获取当前服务的地址。

**返回值：**
- 当前服务地址

**示例：**
```lua
local my_address = skynet.self()
skynet.error("My address:", skynet.address(my_address))
```

### skynet.localname(name)

根据本地名称获取服务地址。

**参数：**
- `name`: 本地服务名称
- 本地服务名列表（skynet提供的本地服务）
    1. .launcher：服务启动器，负责创建和管理所有lua服务。通过 skynet.newservice/uniqueservice 创建服务时，实际是请求 .launcher 来完成服务的加载和初始化。
    2. .service_mgr：服务管理器，管理服务的注册和查找。支持 skynet.name(".xxx") 注册本地服务名，通过 skynet.localname(".xxx") 查找其他本地服务名
    3. .console：调试控制台服务。通常用于 telnet 远程连接，执行调试命令。支持 list/stat/info/kill/mem 等命令，便于开发和运维。
    4. .logger：日志服务。skynet.error(...) 的内容会被发送到 .logger，由它统一处理和输出到文件或终端。
    5. .cslave、.cmaster 等集群相关服务（启用集群才会启动本服务）。负责节点间的注册、发现和通信
    6. 其他系统服务： .debug_agent（远程调试）、.snaxd（snax服务管理）、.sharedatad（共享数据服务）等，根据配置和业务需求自动启动。

**返回值：**
- 服务地址

**示例：**
```lua
local launcher = skynet.localname(".launcher")
skynet.send(launcher, "lua", "somecmd")
```

### skynet.address(service)
Skynet 里的服务地址（handle）本质上是一个内部的数字 ID（如 0x1000001），不是 IP 或端口。
转换后，会变成可读的字符串（如 :01000001），但仍然只是 Skynet 内部的标识，不是网络地址。
将服务地址转换为可读的字符串形式。

**参数：**
- `service`: 服务地址

**返回值：**
- 地址字符串

**示例：**
```lua
local addr_str = skynet.address(service)
skynet.error("Service address:", addr_str)
```

## 📨 消息通信

### skynet.send(address, typename, ...)

发送消息给指定服务，不等待回复。
不阻塞调用者
dispatch接口接受时，默认消息id = 0

**参数：**
- `address`: 目标服务地址
- `typename`: 消息类型
- `...`: 消息参数

**示例：**
```lua
-- 发送方
skynet.send(worker, "lua", "work", data)
skynet.send(logger, "text", "log message")

-- 接收方
skynet.dispatch("lua", function(session, address, cmd, ...)
    -- 不需要回复
    -- session：消息id = 0
    -- address：调用者地址
    -- cmd: 指令
    -- ...: 参数列表
end)
```

### skynet.call(address, typename, ...)

发送消息给指定服务并等待回复。
阻塞调用者，直到被调用服务使用 ret() 返回消息
dispatch接口接受时，默认消息id 为 1 开始的递增值（判断消息id可以判断来源是否需要回复）

**参数：**
- `address`: 目标服务地址
- `typename`: 消息类型
- `msg`: 消息（判断此项来判断调用哪个处理函数）
- `...`: 消息参数

**返回值：**
- 目标服务的回复

**示例：**
```lua
-- 发送方
local result = skynet.call(db, "lua", "query", "SELECT * FROM users")
local player_data = skynet.call(player_mgr, "lua", "get_player", player_id)

-- 接收方
skynet.dispatch("lua", function(session, address, cmd, ...)
    -- session：消息id 自增
    -- address：调用者地址
    -- cmd: 指令
    -- ...: 参数列表

    -- 需要回复：
    local response = "Hello, " .. name .. "!"
    skynet.ret(skynet.pack(response))
end)
```

### skynet.rawcall(address, typename, msg, sz)

发送原始数据消息。

**参数：**
- `address`: 目标服务地址
- `typename`: 消息类型
- `msg`: 消息数据指针
- `sz`: 数据大小

**返回值：**
- 回复消息和大小

**示例：**
```lua
local msg, sz = skynet.rawcall(service, "response", data_ptr, data_size)
```

### skynet.ret(...)

在消息处理函数中回复消息。

**参数：**
- `...`: 回复的数据

**示例：**
```lua
skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "ping" then
        skynet.ret(skynet.pack("pong"))
    end
end)
```

### skynet.retpack(...)

回复消息的简化版本，自动打包数据。

**参数：**
- `...`: 回复的数据

**示例：**
```lua
skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "get_time" then
        skynet.retpack(skynet.time())
    end
end)
```

### skynet.response()

创建一个延迟回复对象。
对于耗时较长的任务：各种io，复杂计算
一般使用 延迟回复，然后 fork 一个新的临时携程（不能调用，用完即毁）来处理
此时被调用者 就可以继续处理其他任务
而调用者 会被继续阻塞，直到得到返回值

如果是 send(不需要回复) 触发的长时间任务，不需要创建 response 对象
可以直接 fork 协程，处理任务，不用返回

**返回值：**
- 回复函数

**示例：**
```lua
skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "async_work" then
        local response = skynet.response()
        
        skynet.fork(function()
            local result = do_async_work(...)
            response(true, result)  -- 相当于 ret(), 返回消息给调用者
        end)
    end
end)
```

### skynet.pack(...)

打包数据用于消息传递。
将要发送的 Lua 数据序列化成二进制格式，方便在服务间传递
元数据（如 发送者地址，消息id等） 由Skynet 框架在消息传递时自动加上，和 pack 无关

**参数：**
- `...`: 要打包的数据

**返回值：**
- 打包后的数据

**示例：**
```lua
local packed = skynet.pack("hello", 123, {a=1, b=2})
skynet.ret(packed)
```

### skynet.unpack(msg, sz)

解包消息数据。

**参数：**
- `msg`: 消息数据
- `sz`: 数据大小

**返回值：**
- 解包后的数据

**示例：**
```lua
local data1, data2, data3 = skynet.unpack(msg, sz)
```

### skynet.dispatch(typename, func)

注册消息处理函数。
处理 call, send, rawcall的调用
每个服务都只有唯一的 dispatch 作为接收消息的入口

**参数：**
- `typename`: 消息类型
- `func`: 处理函数

**示例：**
```lua
skynet.dispatch("lua", function(session, address, cmd, ...)
    if cmd == "work" then
        local data = ...
        process_work(data)
        skynet.ret(skynet.pack("done"))
    end
end)

skynet.dispatch("text", function(session, address, msg)
    skynet.error("Received text:", msg)
end)
```

## ⏰ 定时器

### skynet.timeout(time, func)

设置一次性定时器。

**参数：**
- `time`: 超时时间（单位：0.01秒）
- `func`: 回调函数

**示例：**
```lua
skynet.timeout(100, function()  -- 1秒后执行
    skynet.error("Timer expired")
end)
```

### skynet.starttime()

获取系统启动时间。

**返回值：**
- 启动时间戳

**示例：**
```lua
local start = skynet.starttime()
```

### skynet.time()

获取当前时间（从启动开始的时间）。

**返回值：**
- 当前时间（单位：0.01秒）

**示例：**
```lua
local now = skynet.time()
skynet.error("Current time:", now / 100, "seconds")
```

### skynet.sleep(time)

让当前协程休眠指定时间。
会立即让出cpu，然后进入休眠

**参数：**
- `time`: 休眠时间（单位：0.01秒）

**示例：**
```lua
skynet.sleep(100)  -- 休眠1秒
skynet.error("Wake up!")
```

## 🔄 协程管理

### skynet.fork(func, ...)

创建一个新的协程。
用来创建临时协程，处理任务后销毁
无法被 call 等调用

fork 可以用来创建协程池
fork 协程后，进入休眠，
休眠结束时，检查任务队列是否有任务
如果有任务，则执行任务
    任务结束后，继续进行下一个任务或休眠
如果没有任务，进入下一次休眠 sleep

**参数：**
- `func`: 协程函数
- `...`: 传递给函数的参数

**示例：**
```lua
skynet.fork(function(name)
    skynet.error("Hello from coroutine:", name)
    skynet.sleep(100)
    skynet.error("Coroutine finished")
end, "worker")
```

### skynet.yield()

主动让出CPU，让其他协程运行。

**示例：**
```lua
for i = 1, 1000 do
    heavy_computation()
    if i % 100 == 0 then
        skynet.yield()  -- 定期让出CPU
    end
end
```

### skynet.wait()

等待其他协程唤醒当前协程。

与sleep实现协程池的比较：（建议使用sleep）
主动 sleep + 轮询队列（协程池常用）
优点：实现简单，适合高并发、任务频繁的场景。
缺点：有轮询延迟（sleep 时间），可能会有空转消耗。

skynet.wait + skynet.wakeup（事件驱动）
优点：无轮询延迟，协程挂起后不占用 CPU，任务来时立即唤醒，适合任务不频繁但对响应速度要求高的场景。
缺点：实现稍复杂，需要管理协程对象和唤醒逻辑

**返回值：**
- 唤醒时传递的参数

**示例：**
```lua
-- 协程1
skynet.fork(function()
    local data = skynet.wait()
    skynet.error("Received:", data)
end)

-- 协程2
skynet.fork(function()
    skynet.sleep(100)
    skynet.wakeup(co, "hello")
end)
```

### skynet.wakeup(co, ...)

唤醒指定的协程。
只能唤醒 wait() 挂起的协程
无法唤醒 sleep 的协程

**参数：**
- `co`: 协程对象
- `...`: 传递给协程的参数

**示例：**
```lua
local co = skynet.fork(function()
    return skynet.wait()
end)

skynet.timeout(100, function()
    skynet.wakeup(co, "time's up")
end)
```

## 🌐 网络通信

### skynet.socket API

Skynet提供了完整的socket API用于网络通信。

#### socket.listen(host, port, backlog)

监听指定端口。

**参数：**
- `host`: 监听地址
- `port`: 监听端口
- `backlog`: 监听队列长度

**返回值：**
- 监听socket id

**示例：**
```lua
local socket = require "skynet.socket"

-- 消息回调回调函数：一般每个端口绑定一个
-- 自定义函数，但参数列表是固定的两个
-- socket数据流对于同一个端口，出和入的缓冲区是不一样的
local function accept_8888(id, addr)
    skynet.error("New connection from:", addr)
    socket.start(id)    -- 接管socket链接
    
    while true do
        local data = socket.read(id)    -- 从 入 缓冲区读取数据
        if data then
            socket.write(id, "Echo: " .. data)  -- 写入数据（目标是 出 缓冲区）
        else
            break
        end
    end
    
    socket.close(id)    -- 释放socket链接
end

skynet.start(function()
    local id = socket.listen("0.0.0.0", 8888)   -- 监听到消息
    socket.start(id, accept_8888)    -- 调用accept回调，传入消息：id（由skynet添加 addr）
end)
```

#### socket.connect(host, port)

连接到指定服务器。

**参数：**
- `host`: 服务器地址
- `port`: 服务器端口

**返回值：**
- 连接socket id

**示例：**
```lua
local socket = require "skynet.socket"

skynet.start(function()
    local id = socket.connect("127.0.0.1", 8888)
    socket.write(id, "Hello Server")    -- 写入数据并发送（一般等到缓冲区刷新时发送）
    local response = socket.read(id)    -- 读取返回数据（阻塞协程，直到有消息返回或链接关闭）
    skynet.error("Server response:", response)
    socket.close(id)
end)
```

#### socket.read(id, sz)

从socket读取数据。

##### socket 是流式传输（TCP），用 socket.read(id) 读取时，有可能出现“粘包”或“拆包”问题。
TCP 是字节流，没有消息边界，socket.read(id) 只保证读取到数据，不保证一次就是一条完整的消息。
如果发送端连续发送多条消息，接收端可能一次读到多条消息拼在一起（粘包），也可能只读到一部分消息（拆包）。
解决方法：
需要在协议层自己定义消息边界，比如每条消息前加长度字段，或用特殊分隔符（如 "\n"）。

**参数：**
- `id`: socket id
- `sz`: 消息边界条件（可选）
    + 如果传入数字，则读取对应数量的字节
    + 传入字符串或字符（消息结束符），则读取直到遇到字符

**返回值：**
- 读取的数据

**示例：**
```lua
local data = socket.read(id)        -- 读取所有可用数据
local line = socket.read(id, "\n")  -- 读取一行
local bytes = socket.read(id, 10)   -- 读取10字节
```

#### socket.write(id, data)

向socket写入数据并发送
- 写入后会把数据推送到操作系统的 socket 缓冲区，网络线程会 立即尝试 发送。
- 如果数据量很大或网络繁忙，部分数据可能暂时保留在 Skynet 的缓冲区，等待后续继续发送。
    + 如果写入的数据量超过系统缓冲区大小，Skynet 会先把数据分批放入自己的发送缓冲区，剩余部分暂时保留，等待网络线程后续继续发送
    + Skynet 的缓冲区也会逐渐堆积，write 操作本身不会阻塞，但数据发送速度会变慢。
    + 如果持续写入且网络迟迟无法发送出去，Skynet 的缓冲区会越来越大，最终达到上限（默认 64MB），此时再写入会报错（socket.error），连接可能被关闭
- 对于小数据包，write 后几乎是“立即”发送，但不是严格同步完成，属于异步发送机制。

**参数：**
- `id`: socket id
- `data`: 要写入的数据

**示例：**
```lua
socket.write(id, "Hello World")
socket.write(id, string.pack("I4", 123))    -- 打包数据后发送
```

#### socket.close(id)

关闭socket连接。

**参数：**
- `id`: socket id

**示例：**
```lua
socket.close(id)
```

### HTTP API

#### httpc.request(method, host, url, header, content)

发送HTTP请求。

**参数：**
- `method`: HTTP方法
- `host`: 服务器地址
- `url`: 请求URL
- `header`: 请求头
- `content`: 请求体

**返回值：**
- 响应状态码和响应内容

**示例：**
```lua
local httpc = require "http.httpc"

local ok, status, body = pcall(httpc.request, "GET", "httpbin.org", "/get")
-- ok是 pcall的返回值，表示函数运行结果
-- status表示 request的返回值，表示返回的状态：200，404等
-- body表示返回的响应数据
if ok and status == 200 then
    skynet.error("Response:", body)
end
```

## 💾 数据库

### MySQL API

```lua
local mysql = require "skynet.db.mysql"

-- 连接数据库
local db = mysql.connect({
    host = "127.0.0.1", -- 数据库地址
    port = 3306,    -- 数据库端口
    database = "gamedb",    -- 数据库名
    user = "root",  -- 数据库用户名
    password = "password",  -- 数据库密码
    max_packet_size = 1024 * 1024   -- 单次通信时允许的最大数据包大小（单位：字节）防止一次性查询或插入的数据量过大导致内存溢出或协议错误。超过 max_packet_size时，驱动会报错或自动分包
})

-- 执行查询
-- 第一个参数是 SQL 语句，后面的参数会替换语句中的 ?，防止 SQL 注入。
-- 也可以直接传入完整的 SQL 语句（无参数）
-- 返回值是一个 Lua 表
-- 外层 Lua 表是数组，key 从 1 开始，每个元素是一行数据
-- 内层数据是一个表： key 是数据库字段名，内部数据是 查询到的内容
result = {
    [1] = {
        ["name"] = "张三",
        ["years"] = 5,
    },
    [2] = {
        ["name"] = "李四",
        ["years"] = 5,
    }
}
local result = db:query("SELECT * FROM users WHERE id = ?", 1001)
for _, row in ipairs(result) do
    skynet.error("User:", row.name, row.level)
end

-- 插入数据
db:query("INSERT INTO users (name, level) VALUES (?, ?)", "player1", 10)

-- 关闭连接
db:disconnect()
```

### Redis API

```lua
local redis = require "skynet.db.redis"

-- 连接Redis
local db = redis.connect({
    host = "127.0.0.1", -- 地址
    port = 6379,    -- 端口
    db = 0  --  Redis 数据库编号，
-- Redis 支持多数据库，编号从 0 开始，默认是 0。
-- 每个数据库是相互隔离的 key-value 空间，类似于分区。
-- 通过 db = 0 连接的是第 0 号数据库，db = 1 就是第 1 号数据库。
-- 一般业务只用默认的 0 号库，特殊场景可以用其他编号做隔离。
})

-- 设置值
db:set("player:1001", "data")
db:hset("player:1001:info", "level", 10)
-- 对应的 redis 数据库：
[0] = {
    ["player:1001"] = "data",
    ["player:1001:info"] = {
        ["level"] = 10
    }
}

-- 获取值
local data = db:get("player:1001")
local level = db:hget("player:1001:info", "level")

-- 关闭连接
db:disconnect()
```

## 🌍 集群通信

### cluster.call(node, service, ...)

调用远程节点的服务。

**参数：**
- `node`: 节点名称，skynet根据节点名，从配置文件中获取实际地址
- `service`: 服务名称
- `协议`: 默认为lua，可以省略
- `指令`: 调用的指令
- `...`: 指令参数列表

**返回值：**
- 远程调用结果

**示例：**
```lua
local cluster = require "skynet.cluster"

local result = cluster.call("slave1", "player_manager", "get_player", 1001)
```

### cluster.send(node, service, ...)

发送消息到远程节点。

**参数：**
- `node`: 节点名称，skynet根据节点名，从配置文件中获取实际地址
- `service`: 服务名称
- `协议`: 默认为lua，可以省略
- `指令`: 调用的指令
- `...`: 指令参数列表

**示例：**
```lua
cluster.send("slave2", "battle_service", "start_battle", battle_id)
```

### cluster.query(node, service)

查询远程服务。
查询节点配置文件中保存的地址
如果不存在此服务则返回错误并中止

**参数：**
- `node`: 节点名称
- `service`: 服务名称

**返回值：**
- 远程服务地址

**示例：**
```lua
local remote_service = cluster.query("slave1", "database")
```

### cluster.register(name, addr)

注册本地服务到集群。
在本地节点注册服务名与地址的映射，存储在当前节点的 Skynet 管理表（如 .service_mgr 或集群相关服务）中。它不会自动同步到其他节点。

不同节点之间的服务名同步依赖于 Skynet 集群机制：每个节点只管理自己的注册表，节点间通过 cluster.call/query/send 访问远程服务时，会先查本地配置（如 config.lua 的 cluster 字段），再通过集群协议与目标节点通信，目标节点会查自己的注册表返回服务地址。

如果需要所有节点都能访问某个服务，需在每个节点都执行 cluster.register，或通过配置和启动脚本保证服务名一致。Skynet 不会自动广播注册信息，节点间同步完全依赖于集群协议和配置。

**参数：**
- `name`: 服务名称
- `addr`: 服务地址

**示例：**
```lua
cluster.register("my_service", skynet.self())
```

## ⚙️ 配置管理

### 配置文件示例

```lua
-- config.lua
thread = 4  -- 线程数
harbor = 0  -- 标识当前节点的“集群编号”或“分布式节点ID”。= 0 表示不启用分布式功能
-- > 0 时，Skynet 会自动加载集群相关服务（如 .harbor、.cmaster、.cslave），用于节点间通信和服务发现。

start = "main"  -- 第一个启动的入口函数
bootstrap = "snlua bootstrap"   -- 引导服务，用来完成环境初始化
logger = nil    -- 日志配置
-- logger = "./logs/skynet.log"   -- 指定日志文件路径
debug_console = 8000    -- 远程调试端口（详见 debug console 部分）

-- 节点类型配置（用于区分不同角色的节点，便于业务逻辑和集群管理）
node_type = "master"   -- 当前节点类型，可选值如 master/slave/gateway/logic/db 等
-- 节点角色说明
-- node_type = "master"    -- 主节点，负责调度和管理
-- node_type = "slave"     -- 从节点，负责具体业务处理
-- node_type = "gateway"   -- 网关节点，负责客户端连接
-- node_type = "logic"     -- 逻辑节点，处理游戏核心逻辑
-- node_type = "db"        -- 数据库节点，专门处理数据存储

-- 路径配置
root = "./" -- 项目根路径
luaservice = root.."service/?.lua;"..root.."skynet/service/?.lua"   -- lua 服务脚本查找路径
lualoader = root.."skynet/lualib/loader.lua"   -- lua loader入口
lua_path = root.."lualib/?.lua;"..root.."skynet/lualib/?.lua"        -- lua模块查找路径
lua_cpath = root.."luaclib/?.so;"..root.."skynet/luaclib/?.so"      -- C模块查找路径

-- 集群配置（节点名与地址映射，供cluster.call/query/send使用）
cluster = {
    master = "127.0.0.1:7001",   -- 主节点地址
    slave1 = "127.0.0.1:7002",   -- 从节点1地址
    slave2 = "127.0.0.1:7003"    -- 从节点2地址
}

-- 数据库配置（MySQL，供业务服务require使用）
db_mysql = {
    host = "127.0.0.1",      -- 数据库地址
    port = 3306,              -- 数据库端口
    database = "gamedb",     -- 数据库名
    user = "root",           -- 用户名
    password = "password",   -- 密码
    max_packet_size = 1024 * 1024   -- 单次最大数据包（字节）
}

-- Redis配置（供业务服务require使用）
redis = {
    host = "127.0.0.1",      -- Redis地址
    port = 6379,              -- Redis端口
    db = 0                    -- 数据库编号（0为默认）
}

-- 集群服务注册（本地服务名与集群服务名映射，需在各节点注册）
-- 仅明文文本，可遍历进行实际注册
-- cluster_service = {
--     player_manager = ".player_mgr",   -- 注册本地服务名到集群
--     battle_service = ".battle"         -- 可选，按需注册
-- }

-- 自定义配置（业务参数，可按需扩展）
gameconfig = {
    max_players = 1000,                 -- 最大玩家数
    server_name = "Game Server 1"      -- 服务器名称
}
```

### 读取配置

```lua
-- 读取基础配置
local thread_count = tonumber(skynet.getenv("thread"))
local start_service = skynet.getenv("start")

-- 读取自定义配置
local gameconfig = skynet.getenv("gameconfig")
local max_players = gameconfig.max_players
```

## 🔍 调试工具

### skynet.error(...)

输出错误日志。

**参数：**
- `...`: 日志内容

**示例：**
```lua
skynet.error("Debug info:", player_id, level)
skynet.error("Error occurred in function:", func_name)
```

### debug console

启动调试控制台：
- 原理：
    + 在远程服务器的配置文件中添加配置 debug_console = 端口
    + 此端口即为远程调试服务的链接端口
    + skynet识别到此项后，即会监听此端口用来链接调试服务
    + 在本地控制台使用 操作系统工具 telnet 来链接到远程服务进行调试
        telnet 远程地址 配置的端口

```lua
-- 在配置文件中
debug_console = 8000

-- 连接控制台
telnet 127.0.0.1 8000
```

控制台命令：
- `list`：列出所有服务
- `stat`：显示服务统计信息
- `info`: <service>：显示指定服务详细信息
- `kill`: <service>：杀死指定服务
- `mem`：显示内存使用情况
- `gc`：手动触发 Lua 垃圾回收
- `inject`: <service> <lua_file>：向服务注入并执行 Lua 脚本
- `call`: <service> <lua_code>：在服务内执行一段 Lua 代码
- `logon`: <service>：打开指定服务的日志
- `logoff`: <service>：关闭指定服务的日志
- `help`：显示所有支持的命令及说明

### 性能分析

```lua
-- 启用性能分析
local profile = require "skynet.profile"

profile.start()

-- 业务代码...

profile.stop()
profile.report()  -- 输出性能报告
```

## 🛠️ 工具函数

### 字符串操作

```lua
-- string.split 分割字符串
-- 依照分隔符，分割为数组
local parts = string.split("a,b,c", ",")  -- {"a", "b", "c"}

-- string.ltrim/rtrim/trim 去除空白
local str = string.trim("  hello  ")  -- "hello"
```

### 表操作

```lua
-- table.copy 复制表
local new_table = table.copy(old_table)

-- table.merge 合并表
-- 如果某个 key 在 table1 和 table2 都存在，table2 的值会覆盖 table1 的值
local merged = table.merge(table1, table2)

-- table.keys/values 获取键值
-- 筛选出所有的 key 或 value 然后以数组形式返回
local keys = table.keys(my_table)
local values = table.values(my_table)
```

### 数学函数

```lua
-- 手动设置随机数种子（lua会在启动时自动设置种子，可省略）
math.randomseed(os.time())

-- math.random 随机数（已设置种子）
-- 生成1-100之间的随机数
local num = math.random(1, 100)

-- math.round 四舍五入
local rounded = math.round(3.14159, 2)  -- 3.14
```

### 时间函数

```lua
-- os.date 格式化时间
local timestr = os.date("%Y-%m-%d %H:%M:%S")    -- 格式化当前时间
local timestr = os.date("%Y-%m-%d %H:%M:%S", timestamp) -- 格式化传入的时间戳

-- skynet.time 高精度时间
-- 返回的是服务器启动以来的时间（不是 1970 年开始的绝对时间戳）
-- 精度为 0.01 秒
local precise_time = skynet.time()

-- 获取秒级时间戳（1970 年开始的绝对时间戳）
local precise_time = os.time()
```

### 编码解码

```lua
-- base64编码
local base64 = require "skynet.crypt"
local encoded = base64.base64encode("hello")    -- 加密
local decoded = base64.base64decode(encoded)    -- 解密

-- md5散列
local hash = base64.md5("password") -- 加密

-- URL编码
local encoded_url = base64.urlencode("hello world") -- 加密
local decoded = base64.urldecode(encoded)  -- 解密
```

## 📖 使用示例

### 完整的服务示例

```lua
local skynet = require "skynet" -- 引入 skynet
local socket = require "skynet.socket"  -- 引入 socket

-- 服务状态
local players = {}
local stats = {
    total_connections = 0,  -- 总链接
    current_online = 0  -- 当前在线
}

-- 消息处理
skynet.start(function() -- 服务启动
    skynet.error("Game service started")
    
    -- 注册消息处理器
    skynet.dispatch("lua", function(session, address, cmd, ...)
        if cmd == "add_player" then
            local player_id, player_data = ...
            players[player_id] = player_data
            stats.current_online = stats.current_online + 1
            skynet.retpack("ok")
            
        elseif cmd == "remove_player" then
            local player_id = ...
            players[player_id] = nil
            stats.current_online = stats.current_online - 1
            skynet.retpack("ok")
            
        elseif cmd == "get_stats" then
            skynet.retpack(stats)
            
        elseif cmd == "broadcast" then
            local message = ...
            for player_id, player_data in pairs(players) do
                -- 发送消息给所有玩家
                skynet.send(player_data.agent, "lua", "message", message)
            end
            skynet.retpack("ok")
        else
            skynet.error("Unknown command:", cmd)
            skynet.retpack(nil, "unknown command")
        end
    end)
    
    -- 定时统计
    local function stat_timer()
        skynet.error("Online players:", stats.current_online)
        skynet.timeout(6000, stat_timer)  -- 1分钟后再次执行    -- 定时器
    end
    skynet.timeout(6000, stat_timer)    -- 初次启动定时器
end)
```

---

这份API参考手册涵盖了Skynet框架的主要功能。建议配合[快速入门指南](./skynet_quickstart.md)和[核心概念解析](./skynet_concepts.md)一起学习。

*更新时间：2025年8月22日*
