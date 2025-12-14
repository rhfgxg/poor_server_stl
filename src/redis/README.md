# Redis Server - Redis 集群统一访问层

## 📋 概述

Redis Server 是一个统一的 Redis 访问层，负责管理本地 Redis 和 Redis 集群的连接，为其他服务提供统一的接口。

### 主要功能

- ✅ **统一访问接口**：通过 Socket 为 Skynet 提供 Redis 访问
- ✅ **集群管理**：自动处理 Redis Cluster 的 slot 路由
- ✅ **本地缓存**：支持本地 Redis 单机访问
- ✅ **高性能**：使用 C++ 实现，支持批量操作
- ✅ **自动重连**：网络故障自动恢复

---

## 🏗️ 架构设计

```
┌───────────────────────────────────────────────┐
│             Application Layer                 │
├───────────────────────────────────────────────┤
│                                               │
│  ┌──────────────┐      ┌──────────────────┐ │
│  │ C++ Services │      │ Skynet Services  │ │
│  │  - Gateway   │      │  - Logic         │ │
│  │  - Login     │      │  - Battle        │ │
│  │  - DB        │      └─────────┬────────┘ │
│  └──────┬───────┘                │          │
│         │                        │          │
│         │ 直连                   │ Socket   │
│         ↓                        ↓          │
├───────────────────────────────────────────────┤
│           Redis Access Layer                  │
├───────────────────────────────────────────────┤
│                                               │
│  ┌──────────────────────────────────────────┐│
│  │        Redis Server (Port 8003)          ││
│  │                                          ││
│  │  ┌─────────────────┐  ┌───────────────┐││
│  │  │ RedisClient     │  │ RedisCluster  │││
│  │  │ (Local Redis)   │  │ Client        │││
│  │  └────────┬────────┘  └───────┬───────┘││
│  └───────────│───────────────────│────────┘│
│              │                   │          │
│              ↓                   ↓          │
│  ┌──────────────┐    ┌──────────────────┐  │
│  │ Local Redis  │    │  Redis Cluster   │  │
│  │ (6379)       │    │  (7000-7005)     │  │
│  └──────────────┘    └──────────────────┘  │
└───────────────────────────────────────────────┘
```

---

## 🚀 快速开始

### 1. 编译

```bash
# 在项目根目录
cd build
ninja redis_server
```

### 2. 配置

编辑 `config/cpp/cfg_server/cfg_redis_server.lua`:

```lua
return {
    host = "0.0.0.0",
    port = 8003,
    
    local_redis = {
        host = "127.0.0.1",
        port = 6379
    },
    
    cluster_redis = {
        nodes = {
            "127.0.0.1:7000",
            "127.0.0.1:7001",
            -- ... 更多节点
        }
    }
}
```

### 3. 启动

```bash
# 方法 1：使用脚本（推荐）
bash tools/debug/wsl/start_redis_server.sh

# 方法 2：直接运行
cd build/src/redis
./redis_server

# 方法 3：使用服务管理器
bash tools/debug/wsl/manage_services.sh start
```

### 4. 验证

```bash
# 检查进程
ps aux | grep redis_server

# 检查端口
netstat -an | grep 8003

# 查看日志
tail -f logs/redis/redis_server.log
```

---

## 📡 使用方式

### C++ 服务使用（直连）

```cpp
#include "redis_client.h"

// 本地 Redis
RedisClient local_redis;
local_redis.connect("127.0.0.1", 6379);
local_redis.set_with_expire("session:123", data, 3600s);

// 集群 Redis（通过 Redis Server 或直连）
RedisClusterClient cluster;
cluster.connect({"127.0.0.1:7000", "127.0.0.1:7001"});
cluster.set("player:123", player_data);
```

### Skynet 服务使用（Socket）

```lua
-- skynet_src/service/redis_service.lua
local socket = require "skynet.socket"
local json = require "cjson"

-- 连接到 Redis Server
local fd = socket.open("127.0.0.1", 8003)

-- 发送请求
local function call_redis(method, params)
    local request = json.encode({
        method = method,
        params = params
    })
    
    local header = string.pack(">I4", #request)
    socket.write(fd, header .. request)
    
    local response_header = socket.read(fd, 4)
    local length = string.unpack(">I4", response_header)
    local response_body = socket.read(fd, length)
    
    return json.decode(response_body)
end

-- 使用示例
local result = call_redis("set", {
    redis_type = "CLUSTER",
    key = "player:123",
    value = data,
    ttl = 3600
})
```

---

## 🔧 协议规范

### Socket 消息格式

#### 请求格式

```
[length(4字节)][json_body]

JSON Body:
{
    "method": "set",       // 方法名
    "params": {            // 参数
        "redis_type": "LOCAL" | "CLUSTER",
        "key": "key",
        "value": "value",
        "ttl": 3600        // 可选
    }
}
```

#### 响应格式

```
[length(4字节)][json_body]

JSON Body:
{
    "success": true,       // 是否成功
    "result": "OK",        // 结果数据
    "error": "..."         // 错误信息（可选）
}
```

### 支持的方法

| 方法 | 参数 | 说明 |
|------|------|------|
| `set` | redis_type, key, value, ttl | 设置键值 |
| `get` | redis_type, key | 获取值 |
| `del` | redis_type, key | 删除键 |
| `hset` | redis_type, key, field, value | Hash 设置 |
| `hget` | redis_type, key, field | Hash 获取 |
| `zadd` | redis_type, key, score, member | 有序集合添加 |
| `zrevrange` | redis_type, key, start, stop | 有序集合倒序范围查询 |

---

## 📊 性能特性

### 访问延迟

| 访问方式 | 延迟 | 适用场景 |
|---------|------|---------|
| C++ → 本地 Redis | 1-2ms | 会话、临时缓存 |
| C++ → 集群 Redis | 2-5ms | 玩家数据 |
| Skynet → Redis Server → 集群 Redis | 5-10ms | 共享数据 |

### 吞吐量

- 单机 QPS: 10,000+
- 集群 QPS: 50,000+
- 支持批量操作（Pipeline）

---

## 🗂️ 文件结构

```
src/redis/
├── redis_server.h              # Redis 服务器头文件
├── redis_server.cpp            # Redis 服务器实现
├── main.cpp                    # 入口函数
└── CMakeLists.txt              # CMake 配置

src/common/
├── redis_client.h              # 本地 Redis 客户端
├── redis_client.cpp
├── redis_cluster_client.h      # 集群 Redis 客户端
└── redis_cluster_client.cpp

config/cpp/cfg_server/
├── cfg_redis_server.lua        # Redis Server 配置
└── cfg_redis.lua               # Redis 配置（供其他服务使用）

tools/debug/wsl/
├── start_redis_server.sh       # 启动脚本
├── stop_redis_server.sh        # 停止脚本
└── manage_services.sh          # 服务管理脚本
```

---

## 🐛 故障排查

### 问题 1：无法启动

```bash
# 检查端口占用
netstat -an | grep 8003

# 查看日志
tail -f logs/redis/redis_server.log

# 检查配置文件
cat config/cpp/cfg_server/cfg_redis_server.lua
```

### 问题 2：连接 Redis 失败

```bash
# 检查本地 Redis
redis-cli ping

# 检查集群 Redis
redis-cli -c -p 7000 ping

# 查看 Redis Server 日志
tail -f logs/redis/redis_server.log
```

### 问题 3：Skynet 连接失败

```bash
# 检查 Redis Server 是否运行
ps aux | grep redis_server

# 检查网络连接
telnet 127.0.0.1 8003

# 查看 Skynet 日志
tail -f skynet_src/skynet/logs/skynet.log
```

---

## 📚 相关文档

- [Redis 架构整合说明](../../docunment/architecture/redis_integration_summary.md)
- [Skynet 数据架构兼容方案](../../docunment/architecture/skynet_data_integration.md)
- [Redis 服务器架构总结](../../docunment/architecture/redis_server_summary.md)

---

## 📝 开发计划

- [ ] 支持 Redis Sentinel（哨兵模式）
- [ ] 实现连接池管理
- [ ] 添加 gRPC 接口（给 C++ 服务）
- [ ] 支持 Redis Streams
- [ ] 实现分布式锁
- [ ] 添加监控和统计

---

**创建日期**: 2025-12-13  
**维护者**: 开发团队
