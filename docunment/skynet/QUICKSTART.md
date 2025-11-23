# Skynet Protobuf 集成快速开始

本指南帮助你快速在 Skynet 中集成并使用 Protobuf 与 C++ 服务器通信。

## 快速步骤

### 1. 安装 lua-protobuf

**Windows (推荐使用 LuaRocks):**
```powershell
# 安装 LuaRocks (如果还没有)
# 下载：https://luarocks.org/

# 安装 lua-protobuf
luarocks install lua-protobuf

# 找到生成的 protobuf.dll
luarocks show lua-protobuf

# 复制到 skynet
Copy-Item "C:\LuaRocks\lib\lua\5.x\protobuf.dll" "skynet_src\luaclib\protobuf.dll"
```

**Linux:**
```bash
# 安装 lua-protobuf
luarocks install lua-protobuf

# 或手动编译
git clone https://github.com/starwing/lua-protobuf.git
cd lua-protobuf
make
cp protobuf.so /path/to/skynet_src/luaclib/
```

### 2. 生成 Protobuf Descriptor 文件

```powershell
# Windows
.\tools\proto_make_lua.ps1

# Linux/macOS
chmod +x tools/proto_make_lua.sh
./tools/proto_make_lua.sh
```

这会将 `protobuf/*.proto` 编译为 `protobuf/skynet/src/*.pb` 文件。

### 3. 测试 Proto 库

创建测试脚本 `test_proto.lua`：

```lua
local proto = require "proto"

-- 加载 descriptor
local ok, err = proto.load_descriptor("protobuf/skynet/src/test.pb")
if ok then
    print("? Proto loaded successfully")
else
    print("? Failed to load proto:", err)
end

-- 查看已注册的消息类型
local types = proto.get_types()
print("\nRegistered message types:")
for _, name in ipairs(types) do
    print("  -", name)
end

-- 测试编码/解码（根据你的 proto 文件调整）
-- local data = {id = 123, name = "test"}
-- local bytes = proto.encode("YourMessageType", data)
-- local decoded = proto.decode("YourMessageType", bytes)
-- print("\nEncoded size:", #bytes, "bytes")
-- print("Decoded:", decoded.id, decoded.name)
```

运行测试：
```bash
cd skynet_src
lua test_proto.lua
```

### 4. 启动服务示例

创建 Skynet 配置文件 `config_test.lua`：

```lua
-- Skynet 配置示例
thread = 8
logger = nil
logpath = "."
harbor = 0
start = "main"  -- 启动脚本

-- Lua 代码路径
luaservice = root.."service/?.lua;"..root.."service/?/main.lua"
lualoader = root.."lualib/loader.lua"
lua_path = root.."lualib/?.lua;"..root.."lualib/?/init.lua"
lua_cpath = root.."luaclib/?.so;"..root.."luaclib/?.dll"

-- 网络配置
daemon = nil
```

创建启动脚本 `service/main.lua`：

```lua
local skynet = require "skynet"

skynet.start(function()
    skynet.error("=== Skynet Server Starting ===")
    
    -- 启动逻辑服务器
    local logic = skynet.newservice("logic/main")
    skynet.call(logic, "lua", "start")
    skynet.error("Logic server started:", logic)
    
    -- 启动战斗服务器
    local battle = skynet.newservice("battle/main")
    skynet.call(battle, "lua", "start")
    skynet.error("Battle server started:", battle)
    
    -- 测试战斗服务器
    local room_id = skynet.call(battle, "lua", "create_room", {1001, 1002})
    skynet.error("Created battle room:", room_id)
    
    skynet.call(battle, "lua", "start_battle", room_id)
    skynet.error("Battle started")
    
    -- 查询房间状态
    skynet.sleep(200)  -- 等待 2 秒
    local state = skynet.call(battle, "lua", "get_room_state", room_id)
    skynet.error("Room state:", state.state, "Frame:", state.current_frame)
    
    skynet.error("=== All services started ===")
    skynet.error("Press Ctrl+C to exit")
end)
```

启动 Skynet：
```bash
cd skynet_src/skynet
./skynet ../../config_test.lua
```

## 项目结构

生成的文件结构：

```
poor_server_stl/
├── protobuf/
│   ├── common.proto              # 公共消息定义
│   ├── server_logic.proto        # 逻辑服务器协议
│   ├── server_battle.proto       # 战斗服务器协议
│   └── skynet/                   # Skynet 专用 descriptor 文件
│       ├── common.pb             # ← 由 generate_proto_desc.ps1 生成
│       ├── server_logic.pb
│       └── server_battle.pb
├── skynet_src/
│   ├── lualib/
│   │   └── proto.lua             # ← Protobuf 辅助库
│   ├── luaclib/
│   │   └── protobuf.dll          # ← lua-protobuf 动态库
│   ├── service/
│   │   ├── logic/
│   │   │   └── main.lua          # ← 逻辑服务器
│   │   └── battle/
│   │       └── main.lua          # ← 战斗服务器
│   └── README_proto.md           # ← 详细文档
└── tools/
    ├── generate_proto_desc.ps1   # ← Windows 生成脚本
    └── generate_proto_desc.sh    # ← Linux 生成脚本
```

## 常见问题

### Q: 提示 "module 'protobuf' not found"
**A:** lua-protobuf 未安装或路径不对。检查：
1. `skynet_src/luaclib/` 下是否有 `protobuf.dll` (Windows) 或 `protobuf.so` (Linux)
2. Skynet 配置中 `lua_cpath` 是否包含该路径

### Q: 提示 "Failed to load descriptor"
**A:** Descriptor 文件未生成或路径错误：
1. 运行 `.\tools\generate_proto_desc.ps1` 生成 .pb 文件
2. 确认 `protobuf/skynet/` 目录下有 .pb 文件
3. 检查代码中的路径是否正确（相对于项目根目录）

### Q: 消息编码/解码失败
**A:** 可能原因：
1. 消息类型名称错误（注意包名，如 `logic.LoginRequest`）
2. Descriptor 未加载或加载失败
3. 数据字段类型不匹配

使用 `proto.get_types()` 查看已注册的消息类型。

### Q: 与 C++ 服务器通信失败
**A:** 检查：
1. 确保 C++ 和 Lua 使用相同的 proto 文件版本
2. 网络协议一致（长度前缀、字节序）
3. 消息序列化格式匹配

## 下一步

1. **编写业务逻辑**：修改 `service/logic/main.lua` 和 `service/battle/main.lua`
2. **定义协议**：在 `protobuf/` 下创建你的 proto 文件
3. **实现通信**：使用 `proto.encode/decode` 与 C++ 服务器交互
4. **测试**：编写测试用例验证功能

## 参考文档

- **详细文档**: `skynet_src/README_proto.md`
- **逻辑服务器**: `skynet_src/service/logic/main.lua`
- **战斗服务器**: `skynet_src/service/battle/main.lua`
- **Proto 助手**: `skynet_src/lualib/proto.lua`

---

**需要帮助？** 查看详细文档或检查示例代码中的注释。
