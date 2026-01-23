# Redis 架构简化说明

## 更改日期
2025-12-16

## 更改概述

移除 Redis 集群相关代码，简化为 **本地 Redis + MySQL** 架构。

---

## 当前架构

```
┌─────────────────────────────────────────────────────────────┐
│                       Skynet 服务                           │
│  (player_agent, match_service, etc.)                       │
└─────────────────────────────────────────────────────────────┘
                    │                       │
                    ▼                       ▼
        ┌───────────────────┐     ┌───────────────────┐
        │   本地 Redis      │     │   C++ DBServer    │
        │   (缓存热数据)    │     │   (持久化存储)    │
        │   Port: 6379      │     │   Socket: 50053   │
        └───────────────────┘     └───────────────────┘
                                           │
                                           ▼
                                  ┌───────────────────┐
                                  │      MySQL        │
                                  │   (持久化存储)    │
                                  │   Port: 33060     │
                                  └───────────────────┘
```

---

## 数据流

### 读取流程
1. Skynet 先查询本地 Redis（缓存）
2. 缓存未命中时，查询 MySQL
3. 查询结果写入 Redis 缓存

### 写入流程
1. Skynet 写入本地 Redis（缓存）
2. 同时/异步写入 MySQL（持久化）

---

## 修改的文件

### 已修改
| 文件 | 修改内容 |
|------|----------|
| `src/redis/redis_server.h` | 移除集群相关成员和方法 |
| `src/redis/redis_server.cpp` | 移除集群初始化和操作代码 |
| `src/redis/CMakeLists.txt` | 移除 `redis_cluster_client.cpp` |

### 保留但暂不使用
| 文件 | 说明 |
|------|------|
| `src/common/redis_cluster_client.h` | 已添加废弃注释，供未来扩展 |
| `src/common/redis_cluster_client.cpp` | 已添加废弃注释，供未来扩展 |

### 保留使用
| 文件 | 说明 |
|------|------|
| `src/common/redis_client.h` | 本地 Redis 客户端 |
| `src/common/redis_client.cpp` | 本地 Redis 客户端实现 |

---

## API 变化

### Socket 接口（简化后）

#### SET 操作
```json
// 请求
{
    "method": "set",
    "params": {
        "key": "player:1001",
        "value": "{\"name\":\"test\"}",
        "ttl": 3600
    }
}

// 响应
{
    "success": true,
    "result": "OK"
}
```

#### GET 操作
```json
// 请求
{
    "method": "get",
    "params": {
        "key": "player:1001"
    }
}

// 响应
{
    "success": true,
    "result": "{\"name\":\"test\"}"
}
```

#### DEL 操作
```json
// 请求
{
    "method": "del",
    "params": {
        "key": "player:1001"
    }
}

// 响应
{
    "success": true,
    "result": "OK"
}
```

**注意**：移除了 `redis_type` 参数，所有操作默认使用本地 Redis。

---

## 未来扩展

如需支持 Redis 集群，可：

1. 重新启用 `redis_cluster_client.h/cpp`
2. 在 `redis_server.h` 中恢复集群相关成员
3. 在 `redis_server.cpp` 中恢复集群初始化和操作
4. 在 CMakeLists.txt 中添加 `redis_cluster_client.cpp`

---

## 优势

1. **简化架构** - 减少维护复杂度
2. **降低学习成本** - 专注于核心业务逻辑
3. **保留扩展性** - 集群代码保留，随时可启用
4. **本地开发友好** - 只需运行单个 Redis 实例

---

## 配置

### 本地 Redis 配置
文件：`config/cpp/cfg_server/cfg_redis.lua`

```lua
return {
    local = {
        host = "127.0.0.1",
        port = 6379,
        db = 0
    }
}
```

---

## 相关文档

- [Skynet DBServer 集成](../skynet/Read_Operation_Success.md)
- [数据存储架构](./Data_Storage_Architecture.md)
