# Skynet Protobuf 集成指南

本文档说明如何在 Skynet 中集成和使用 Protobuf 进行与 C++ 服务器的通信。

## 一、安装 lua-protobuf

Skynet 使用 [lua-protobuf](https://github.com/starwing/lua-protobuf) 库来支持 protobuf 协议。

### 方法 1: 通过 LuaRocks 安装（推荐）

```powershell
luarocks install lua-protobuf
```

安装后，将生成的 `protobuf.dll` (Windows) 或 `protobuf.so` (Linux) 复制到：
- `skynet_src/luaclib/protobuf.dll` (Windows)
- `skynet_src/luaclib/protobuf.so` (Linux)

### 方法 2: 手动编译

1. 克隆仓库：
```powershell
git clone https://github.com/starwing/lua-protobuf.git
cd lua-protobuf
```

2. Windows 编译（需要 Visual Studio）：
```powershell
# 使用 CMake
mkdir build; cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

3. Linux 编译：
```bash
make
```

4. 复制编译产物到 `skynet_src/luaclib/`

## 二、生成 Protobuf Descriptor 文件

Skynet 使用 descriptor 文件（.pb）而不是 .proto 文本文件。需要用 `protoc` 生成。

### 使用提供的脚本（推荐）

项目提供了自动化脚本：

```powershell
# Windows
.\tools\generate_proto_desc.ps1

# Linux
./tools/generate_proto_desc.sh
```

脚本会自动：
1. 读取 `protobuf/*.proto` 文件
2. 生成 descriptor 文件到 `protobuf/desc/*.pb`
3. 保持目录结构

### 手动生成

```powershell
# 为单个 proto 文件生成 descriptor
protoc --descriptor_set_out=protobuf/skynet/server_logic.pb --proto_path=protobuf protobuf/server_logic.proto

# 批量生成（包含依赖）
protoc --descriptor_set_out=protobuf/skynet/all.pb `
       --include_imports `
       --proto_path=protobuf `
       protobuf/*.proto
```

**重要参数说明：**
- `--descriptor_set_out`: 输出 descriptor 文件路径
- `--include_imports`: 包含所有依赖的 proto 文件（如 common.proto）
- `--proto_path`: proto 文件搜索路径

## 三、在 Skynet 服务中使用 Protobuf

### 1. 基本用法

```lua
local skynet = require "skynet"
local proto = require "proto"

skynet.start(function()
    -- 加载 descriptor 文件
    local ok, err = proto.load_descriptor("protobuf/skynet/server_logic.pb")
    if not ok then
        skynet.error("Failed to load proto:", err)
        return
    end
    
    -- 编码消息
    local request_data = {
        user_id = 12345,
        token = "abc123"
    }
    local bytes = proto.encode("logic.LoginRequest", request_data)
    
    -- 解码消息
    local response = proto.decode("logic.LoginResponse", response_bytes)
    skynet.error("User ID:", response.user_id)
end)
```

### 2. 与 C++ 服务器通信示例

```lua
local skynet = require "skynet"
local socket = require "skynet.socket"
local proto = require "proto"

local function send_to_cpp_server(fd, msg_name, data)
    -- 编码 protobuf 消息
    local body = proto.encode(msg_name, data)
    
    -- 发送消息（假设使用长度前缀协议）
    local len = #body
    local header = string.pack(">I4", len)  -- 4字节大端长度
    socket.write(fd, header .. body)
end

local function recv_from_cpp_server(fd, msg_name)
    -- 读取长度
    local header = socket.read(fd, 4)
    if not header then
        return nil
    end
    local len = string.unpack(">I4", header)
    
    -- 读取消息体
    local body = socket.read(fd, len)
    if not body then
        return nil
    end
    
    -- 解码
    return proto.decode(msg_name, body)
end

skynet.start(function()
    proto.load_descriptor("protobuf/skynet/server_gateway.pb")
    
    -- 连接到 C++ 网关服务器
    local fd = socket.open("127.0.0.1", 8888)
    
    -- 发送登录请求
    send_to_cpp_server(fd, "gateway.LoginRequest", {
        username = "test_user",
        password = "password123"
    })
    
    -- 接收响应
    local response = recv_from_cpp_server(fd, "gateway.LoginResponse")
    if response then
        skynet.error("Login result:", response.success)
    end
end)
```

## 四、项目结构

```
poor_server_stl/
├── protobuf/                    # Proto 定义文件
│   ├── common.proto            # 公共消息定义
│   ├── server_logic.proto      # 逻辑服务器消息
│   ├── server_battle.proto     # 战斗服务器消息
│   └── skynet/                 # Skynet 专用 descriptor 文件（.pb）
│       ├── common.pb
│       ├── server_logic.pb
│       └── server_battle.pb
├── skynet_src/
│   ├── lualib/
│   │   └── proto.lua           # Protobuf 辅助库
│   ├── luaclib/
│   │   └── protobuf.dll        # lua-protobuf 动态库
│   └── service/
│       ├── logic/              # 逻辑服务器
│       │   └── main.lua
│       └── battle/             # 战斗服务器
│           └── main.lua
└── tools/
    └── generate_proto_desc.ps1 # 生成 descriptor 脚本
```

## 五、常见问题

### Q1: 提示 "module 'protobuf' not found"
**A:** lua-protobuf 未正确安装。检查 `skynet_src/luaclib/` 下是否有 `protobuf.dll` 或 `protobuf.so`。

### Q2: 提示 "Failed to decode message"
**A:** 可能原因：
1. Descriptor 文件未正确加载
2. 消息类型名称不匹配（注意包名，如 `logic.LoginRequest`）
3. C++ 和 Lua 使用的 proto 文件版本不一致

### Q3: 如何调试 protobuf 消息？
**A:** 使用以下方法：
```lua
-- 查看所有已注册的消息类型
local types = proto.get_types()
for _, name in ipairs(types) do
    skynet.error("Message type:", name)
end

-- 检查特定类型是否存在
if proto.has_type("logic.LoginRequest") then
    skynet.error("Type exists")
end
```

### Q4: 如何处理嵌套消息和 import？
**A:** 使用 `--include_imports` 参数生成 descriptor：
```powershell
protoc --descriptor_set_out=protobuf/skynet/output.pb --include_imports --proto_path=protobuf protobuf/*.proto
```
这会将所有依赖（如 common.proto）一起打包到 descriptor 中。

## 六、性能优化建议

1. **预加载 descriptor**：在服务启动时一次性加载所有需要的 descriptor，避免运行时加载。

2. **复用消息对象**：对于高频消息，可以复用 table 对象减少 GC 压力。

3. **批量处理**：将多个小消息合并成一个大消息发送，减少网络开销。

4. **使用连接池**：对于频繁通信的 C++ 服务器，使用 socket 连接池避免频繁建立连接。

## 七、下一步

1. 查看 `skynet_src/service/logic/main.lua` 了解逻辑服务器实现
2. 查看 `skynet_src/service/battle/main.lua` 了解战斗服务器实现
3. 根据实际需求修改 proto 文件并重新生成 descriptor
4. 实现与 C++ 数据库服务器、网关服务器的通信逻辑

## 参考资料

- [lua-protobuf GitHub](https://github.com/starwing/lua-protobuf)
- [Skynet 官方文档](https://github.com/cloudwu/skynet/wiki)
- [Protocol Buffers 官方文档](https://protobuf.dev/)
