# Skynet Protobuf 集成完成

## ? 已完成的工作

### 1. Proto 辅助库
- **文件**: `skynet_src/lualib/proto.lua`
- **功能**: 
  - 加载 protobuf descriptor 文件
  - 编码/解码消息
  - 类型检查和管理

### 2. 服务示例
- **逻辑服务器**: `skynet_src/service/logic/main.lua`
  - 玩家登录/登出处理
  - 与数据库服务器通信
  - 玩家会话管理
  
- **战斗服务器**: `skynet_src/service/battle/main.lua`
  - 战斗房间管理
  - 战斗帧循环
  - 玩家操作处理

### 3. 工具脚本
- **Windows**: `tools/generate_proto_desc.ps1`
- **Linux**: `tools/generate_proto_desc.sh`
- **功能**: 自动将 .proto 文件编译为 .pb descriptor 文件
- **特性**: 自动检测 vcpkg 安装的 protoc

### 4. 文档
- **快速开始**: `skynet_src/QUICKSTART.md`
- **详细文档**: `skynet_src/README_proto.md`
- **测试脚本**: `skynet_src/test_proto.lua`

### 5. 测试文件
- **Proto 文件**: `protobuf/test.proto`
- **Descriptor**: `protobuf/skynet/test.pb` ? 已生成

## 下一步操作

### 1. 安装 lua-protobuf（必需）

**方法 A: 使用 LuaRocks（推荐）**
```powershell
luarocks install lua-protobuf
# 将生成的 protobuf.dll 复制到 skynet_src/luaclib/
```

**方法 B: 手动编译**
```powershell
git clone https://github.com/starwing/lua-protobuf.git
cd lua-protobuf
# 使用 CMake 或 make 编译
# 复制 protobuf.dll 到 skynet_src/luaclib/
```

### 2. 测试 Proto 集成

```powershell
cd skynet_src
lua test_proto.lua
```

应该看到类似输出：
```
=== Skynet Protobuf Integration Test ===

1. Loading protobuf module...
? Proto module loaded

2. Loading descriptor file...
? Descriptor loaded: protobuf/skynet/test.pb

3. Registered message types:
  - skynet_test.TestRequest
  - skynet_test.TestResponse
  ...

? All tests passed!
```

### 3. 定义你的协议

根据 C++ 服务器的协议，编写对应的 .proto 文件：

**示例**: `protobuf/server_logic.proto`
```protobuf
syntax = "proto3";
package logic;

message LoginRequest {
    int64 user_id = 1;
    string token = 2;
}

message LoginResponse {
    bool success = 1;
    string message = 2;
    PlayerData player_data = 3;
}

message PlayerData {
    int64 user_id = 1;
    string username = 2;
    int32 level = 3;
    // ... 更多字段
}
```

### 4. 生成 Descriptor

```powershell
.\tools\generate_proto_desc.ps1
```

### 5. 配置 Skynet

创建配置文件 `skynet_src/config.lua`:

```lua
thread = 8
harbor = 0
start = "main"
bootstrap = "snlua bootstrap"

luaservice = root.."service/?.lua;"..root.."service/?/main.lua"
lualoader = root.."lualib/loader.lua"
lua_path = root.."lualib/?.lua;"..root.."lualib/?/init.lua"
lua_cpath = root.."luaclib/?.so;"..root.."luaclib/?.dll"

cpath = root.."cservice/?.so"
```

### 6. 启动服务

```bash
cd skynet_src/skynet
./skynet ../config.lua
```

## 项目架构

```
C++ 服务器 (已有)          Skynet 服务器 (新)
┌─────────────────┐        ┌──────────────────┐
│  Database       │←──────→│  Logic Service   │
│  (C++)          │ Proto  │  (Lua)           │
└─────────────────┘        └──────────────────┘
                                    ↑
┌─────────────────┐                │ Skynet
│  Gateway        │                │ 内部通信
│  (C++)          │                ↓
└─────────────────┘        ┌──────────────────┐
        ↑                  │  Battle Service  │
        │                  │  (Lua)           │
        │                  └──────────────────┘
    客户端
```

## 通信流程示例

### 玩家登录流程

1. **客户端 → C++ Gateway**: 发送登录请求（Protobuf）
2. **C++ Gateway → Skynet Logic**: 转发登录请求（Protobuf）
3. **Skynet Logic → C++ Database**: 查询玩家数据（Protobuf）
4. **C++ Database → Skynet Logic**: 返回玩家数据（Protobuf）
5. **Skynet Logic → C++ Gateway**: 返回登录响应（Protobuf）
6. **C++ Gateway → 客户端**: 返回登录结果（Protobuf）

### Lua 代码示例

```lua
-- 在 logic/main.lua 中
local function handle_login_from_gateway(fd, request_bytes)
    -- 解码来自网关的请求
    local req = proto.decode("logic.LoginRequest", request_bytes)
    
    -- 向数据库查询玩家数据
    if connections.db then
        local db_req = {user_id = req.user_id}
        send_proto_msg(connections.db, "db.QueryPlayerRequest", db_req)
        
        local db_resp = recv_proto_msg(connections.db, "db.QueryPlayerResponse")
        
        -- 构造响应
        local response = {
            success = db_resp.found,
            player_data = db_resp.player_data
        }
        
        -- 发送回网关
        send_proto_msg(fd, "logic.LoginResponse", response)
    end
end
```

## 常见问题解决

### ? "module 'protobuf' not found"
- 未安装 lua-protobuf
- `protobuf.dll` 不在正确路径
- **解决**: 参考 `QUICKSTART.md` 第 1 步

### ? "Failed to load descriptor"
- Descriptor 文件未生成
- 路径错误
- **解决**: 运行 `.\tools\generate_proto_desc.ps1`

### ? "Unknown message type"
- 消息类型名称错误（注意包名）
- Descriptor 未正确加载
- **解决**: 使用 `proto.get_types()` 查看所有类型

## 参考资料

- **Skynet 文档**: https://github.com/cloudwu/skynet/wiki
- **lua-protobuf**: https://github.com/starwing/lua-protobuf
- **Protocol Buffers**: https://protobuf.dev/

## 技术支持

遇到问题时：
1. 查看 `skynet_src/README_proto.md` 详细文档
2. 运行 `lua skynet_src/test_proto.lua` 诊断
3. 检查 Skynet 日志输出
4. 使用 `proto.get_types()` 验证消息类型

---

**状态**: ? 集成完成，等待测试
**下一步**: 安装 lua-protobuf 并运行测试
