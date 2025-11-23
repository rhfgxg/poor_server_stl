# 目录结构更新说明

## 变更内容

为了更清晰地区分 Skynet（Lua）和 C++（gRPC）使用的 Protobuf 文件，已将目录结构调整如下：

### 之前
```
protobuf/
├── *.proto              # Proto 定义文件
└── desc/                # 生成的 descriptor 文件（不明确用途）
    └── *.pb
```

### 现在
```
protobuf/
├── *.proto              # 公共 Proto 定义文件
├── cpp/                 # C++ 服务器专用（已存在）
│   ├── *.proto          # C++ gRPC 专用 proto
│   └── src/             # C++ 生成的代码
│       └── *.pb.cc, *.pb.h
└── skynet/              # Skynet 服务器专用（新）
    └── *.pb             # Lua protobuf descriptor 文件
```

## 优势

1. **清晰的职责分离**
   - `protobuf/cpp/` - C++ 服务器使用 gRPC + protobuf
   - `protobuf/skynet/` - Skynet 服务器使用 lua-protobuf

2. **避免混淆**
   - 一目了然哪些文件用于哪个服务器
   - 便于管理不同的生成产物

3. **符合项目现状**
   - 项目已有 `protobuf/cpp/` 目录
   - 新增 `protobuf/skynet/` 保持一致性

## 已更新的文件

所有引用路径的文件已自动更新：

- ? `tools/generate_proto_desc.ps1` - Windows 生成脚本
- ? `tools/generate_proto_desc.sh` - Linux 生成脚本
- ? `skynet_src/lualib/proto.lua` - Proto 辅助库
- ? `skynet_src/service/logic/main.lua` - 逻辑服务器
- ? `skynet_src/service/battle/main.lua` - 战斗服务器
- ? `skynet_src/test_proto.lua` - 测试脚本
- ? `skynet_src/README_proto.md` - 详细文档
- ? `skynet_src/QUICKSTART.md` - 快速开始
- ? `skynet_src/INTEGRATION_SUMMARY.md` - 集成总结

## 使用方法

### 生成 Skynet Descriptor 文件

```powershell
# Windows
.\tools\generate_proto_desc.ps1

# 输出到: protobuf/skynet/*.pb
```

### 在代码中使用

```lua
local proto = require "proto"

-- 加载 Skynet 专用的 descriptor
proto.load_descriptor("protobuf/skynet/server_logic.pb")
proto.load_descriptor("protobuf/skynet/common.pb")
```

## 迁移说明

如果你有旧代码引用 `protobuf/desc/`，请更新为 `protobuf/skynet/`：

```lua
-- 旧路径
proto.load_descriptor("protobuf/desc/server_logic.pb")

-- 新路径
proto.load_descriptor("protobuf/skynet/server_logic.pb")
```

## 验证

运行测试脚本验证更改：

```powershell
cd skynet_src
lua test_proto.lua
```

应该看到：
```
? Descriptor loaded: protobuf/skynet/test.pb
? All tests passed!
```

---

**更新日期**: 2025年11月22日  
**影响范围**: 仅路径引用，功能无变化
