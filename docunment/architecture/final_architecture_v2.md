# Poor Server STL - 最终架构设计

> **版本**: v2.0  
> **更新日期**: 2025年1月  
> **架构类型**: C++ 基础服务 + Skynet 游戏逻辑

---

## 🏗️ 架构总览

### 核心设计理念

```
✅ C++ 做擅长的事：
  - 高性能网络 I/O
  - 数据库操作
  - 文件服务
  - 基础框架

✅ Skynet/Lua 做擅长的事：
  - 游戏逻辑
  - 玩家状态管理
  - 战斗系统
  - 热更新
```

---

## 📐 系统架构图

```
┌─────────────────────────────────────────────────────────┐
│                        客户端                            │
│              (Unity/Web/Mobile)                         │
└────────────────────┬────────────────────────────────────┘
                     │ WebSocket/TCP
                     ↓
┌────────────────────────────────────────────────────────┐
│              Gateway Server (C++)                       │
│  ┌──────────────────────────────────────────────┐      │
│  │  连接管理  │  协议解析  │  消息路由          │      │
│  └──────────────────────────────────────────────┘      │
└─────┬──────┬──────┬──────┬────────────────────────────┘
      │      │      │      │
      │      │      │      └─────────────┐
      ↓      ↓      ↓      ↓             ↓
   Central  DB    File   Login    Skynet Logic
   (C++)  (C++)  (C++)  (C++)        (Lua)
                                        │
                          ┌─────────────┼─────────────┐
                          │             │             │
                      Player       Hearthstone    其他游戏
                      Manager      Manager        逻辑模块
                          │             │
                    ┌─────┴─────┐   ┌──┴──┐
                  Agent Agent  ...  Battle
                  (玩家1)(玩家2)     System
```

---

## 🔄 服务职责划分

### 1. Gateway Server (C++)

**职责**：
- ✅ 客户端连接管理（WebSocket/TCP）
- ✅ 协议解析（Protobuf）
- ✅ **消息路由到不同服务**
- ✅ 会话管理

**与 Skynet 通信**：
```cpp
// Gateway 直接通过 TCP 发送消息到 Skynet
// 消息格式：[length(4)][type(2)][data]

// 示例：玩家进入游戏
void Gateway::HandleEnterGame(player_id, game_type) {
    // 构造消息
    SkynetMessage msg;
    msg.type = MSG_ENTER_GAME;
    msg.data = serialize(player_id, game_type);
    
    // 发送到 Skynet
    skynet_conn_->SendMessage(msg);
}
```

---

### 2. Central Server (C++)

**职责**：
- ✅ 服务注册与发现
- ✅ 心跳检测
- ✅ 服务状态管理
- ✅ **Skynet 服务状态监控**

**Skynet 集成**：
```cpp
// Central 管理 Skynet 服务
servers_.push_back({
    .type = ServerType::LOGIC,
    .name = "skynet_logic",
    .address = "127.0.0.1:8888",
    .protocol = "SKYNET_TCP"
});
```

---

### 3. DB Server (C++)

**职责**：
- ✅ 数据库读写（MySQL）
- ✅ 缓存管理（Redis）
- ✅ **为 Skynet 提供数据服务**（通过 gRPC）

**Skynet 调用 DB**：
```lua
-- Skynet 服务通过 gRPC 调用 DB Server
local db_client = require "grpc_client"
local player_data = db_client.call("db_server", "GetPlayerData", {
    player_id = player_id
})
```

---

### 4. Skynet Logic (Lua)

**职责**：
- ✅ **所有游戏逻辑**
- ✅ 玩家状态管理
- ✅ 战斗系统
- ✅ 匹配系统
- ✅ 热更新

**服务结构**：
```
skynet_src/service/
├── main.lua                    # 启动脚本
├── cpp_gateway.lua             # 接收 C++ 消息
├── player_manager.lua          # 玩家管理器
└── logic/
    ├── player_agent.lua        # 玩家代理（每玩家一个）
    ├── hearthstone_manager.lua # 炉石传说管理器
    ├── hearthstone_battle.lua  # 战斗服务
    └── ... 其他游戏模块
```

---

## 🔌 通信协议

### Gateway ↔ Skynet

#### 连接方式
```
Gateway (C++)  →  TCP 8888  →  Skynet cpp_gateway.lua
```

#### 消息格式
```
┌────────────┬────────────┬────────────┐
│  Length    │   Type     │    Data    │
│  (4 bytes) │ (2 bytes)  │ (n bytes)  │
└────────────┴────────────┴────────────┘

Length: Type + Data 的总长度
Type:   消息类型 (1=ENTER_GAME, 2=LEAVE_GAME, 3=ACTION, ...)
Data:   消息数据（Protobuf 序列化或 JSON）
```

#### 示例

```cpp
// C++ 端发送
uint32_t length = 2 + data.size();  // type(2) + data
uint16_t type = 1;  // ENTER_GAME
send_bytes(length, type, data);

// Lua 端接收
local function parse_message(buffer)
    local length = read_uint32(buffer, 1)
    local type = read_uint16(buffer, 5)
    local data = string.sub(buffer, 7, 6 + length - 2)
    return type, data
end
```

---

### C++ 服务之间：gRPC

```
Central ←→ Gateway ←→ DB ←→ File  (gRPC)
```

---

## 🎮 游戏流程示例

### 玩家进入炉石传说

```
1. 客户端 → Gateway
   └─ WebSocket: { type: "ENTER_GAME", game: "hearthstone", player_id: "123" }

2. Gateway → Skynet
   └─ TCP: [length][type=1][data={ player_id: "123", game: "hearthstone" }]

3. Skynet cpp_gateway.lua 接收
   └─ 解析消息，转发到 player_manager

4. player_manager.lua
   └─ 创建或获取 player_agent (player_123)

5. player_agent.lua
   └─ 加载玩家数据（调用 DB Server gRPC）
   └─ 初始化玩家状态
   └─ 返回成功响应

6. Skynet → Gateway
   └─ TCP: [length][type=101][data={ success: true, service: "player_123" }]

7. Gateway → 客户端
   └─ WebSocket: { success: true, message: "进入游戏成功" }
```

---

## 🛠️ 开发工作流

### 修改游戏逻辑（Lua）

```bash
# 1. 修改 Lua 代码
vim skynet_src/service/logic/player_agent.lua

# 2. 热更新（无需重启）
# 在 Skynet 调试控制台执行
> reload("logic.player_agent")

# 或重启 Skynet
pkill skynet
bash tools/skynet/start_skynet.sh
```

### 修改基础服务（C++）

```bash
# 1. 修改 C++ 代码
vim src/gateway/server/gateway_server.cpp

# 2. 重新编译
cd build
ninja gateway

# 3. 重启服务
pkill gateway
./src/gateway/gateway
```

---

## 📊 性能优势

### 对比旧架构（C++ Logic 转发）

```
旧架构：
客户端 → Gateway → Logic(C++) → Skynet → 处理 → 返回
                    ↑ 多一层转发

新架构：
客户端 → Gateway → Skynet → 处理 → 返回
                   ↑ 直连，减少延迟
```

**性能提升**：
- ✅ 减少 1 次网络跳转
- ✅ 减少 1 次消息序列化/反序列化
- ✅ 简化代码，减少 Bug
- ✅ 降低资源消耗

---

## 🔐 安全性

### Gateway 层面

```cpp
// 1. 身份验证
bool Gateway::Authenticate(const string& token) {
    // 验证 JWT token
    return jwt::verify(token);
}

// 2. 消息频率限制
class RateLimiter {
    bool CheckLimit(player_id, msg_type) {
        // 每秒最多 10 条消息
        return counter_[player_id][msg_type] < 10;
    }
};

// 3. 数据校验
bool Gateway::ValidateMessage(const Message& msg) {
    // 校验消息格式、长度等
    return msg.size() < MAX_SIZE && msg.valid();
}
```

### Skynet 层面

```lua
-- 1. 玩家权限检查
local function check_permission(player_id, action)
    local player = get_player(player_id)
    if not player:has_permission(action) then
        return false, "permission denied"
    end
    return true
end

-- 2. 数据校验
local function validate_action(action_data)
    if not action_data or type(action_data) ~= "table" then
        return false, "invalid data"
    end
    return true
end
```

---

## 📈 可扩展性

### 水平扩展

```
            Load Balancer
                  │
      ┌───────────┼───────────┐
      ↓           ↓           ↓
  Gateway1    Gateway2    Gateway3
      │           │           │
      └───────────┼───────────┘
                  ↓
         Skynet Logic Cluster
            (多个节点)
```

### 垂直扩展

```lua
-- 增加 Skynet 工作线程
-- config/skynet/config.lua
thread = 16  -- 从 8 增加到 16
```

---

## 🔍 监控和调试

### Skynet 调试控制台

```bash
# 连接到 Skynet 调试控制台
telnet 127.0.0.1 8000

# 查看所有服务
> list

# 查看服务详情
> info :01000003

# 热更新模块
> reload("logic.player_agent")

# 杀死服务
> kill :01000003
```

### 日志查看

```bash
# Skynet 日志
tail -f logs/skynet.log

# Gateway 日志
tail -f logs/gateway.log

# 所有日志
tail -f logs/*.log
```

---

## 📚 相关文档

- [Skynet 快速入门](skynet_quickstart.md)
- [Skynet API 参考](skynet_api_reference.md)
- [Gateway 开发指南](../server/server_gateway.md)
- [消息协议规范](../protocol/message_protocol.md)

---

## 🎯 总结

### 核心优势

1. ✅ **职责清晰**：C++ 做基础，Lua 做逻辑
2. ✅ **性能优化**：减少转发层次
3. ✅ **开发效率**：Lua 快速迭代
4. ✅ **热更新**：无需停服
5. ✅ **易于维护**：代码结构清晰

### 下一步

- [ ] 实现 Gateway → Skynet 连接模块
- [ ] 完善 Skynet 服务（数据库集成、Redis 等）
- [ ] 实现炉石传说核心玩法
- [ ] 压力测试和性能优化

---

**最后更新**: 2025年1月  
**架构版本**: v2.0
