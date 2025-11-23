# Skynet 集成项目结构说明

本文档说明 Skynet Protobuf 集成后的完整项目结构。

## 目录结构概览

```
poor_server_stl/
├── protobuf/                      # Protobuf 协议文件
│   ├── cpp/                       # C++ 服务器专用
│   │   ├── *.proto                # C++ gRPC proto 定义
│   │   └── src/                   # C++ 生成的代码
│   │       ├── *.pb.cc            # Protobuf 实现
│   │       ├── *.pb.h             # Protobuf 头文件
│   │       └── *.grpc.pb.*        # gRPC 生成代码
│   │
│   └── skynet/                    # Skynet 服务器专用 ⭐ 新增
│       ├── *.proto                # Skynet proto 定义
│       ├── src/                   # 生成的 descriptor 文件
│       │   └── *.pb               # lua-protobuf descriptor
│       └── README.md              # Skynet proto 使用说明
│
├── skynet_src/                    # Skynet 源代码
│   ├── lualib/                    # Lua 库
│   │   └── proto.lua              # Protobuf 辅助库 ⭐ 新增
│   │
│   ├── luaclib/                   # Lua C 扩展
│   │   └── protobuf.dll/.so       # lua-protobuf 库（需安装）
│   │
│   ├── service/                   # Skynet 服务
│   │   ├── logic/                 # 逻辑服务器 ⭐ 新增
│   │   │   └── main.lua
│   │   └── battle/                # 战斗服务器 ⭐ 新增
│   │       └── main.lua
│   │
│   ├── skynet/                    # Skynet 框架
│   └── test_proto.lua             # Proto 测试脚本 ⭐ 新增
│
├── docunment/                     # 项目文档
│   └── skynet/                    # Skynet 相关文档 ⭐ 新增
│       ├── README_proto.md        # Protobuf 详细文档
│       ├── QUICKSTART.md          # 快速开始指南
│       ├── INTEGRATION_SUMMARY.md # 集成总结
│       └── DIRECTORY_STRUCTURE_UPDATE.md  # 目录结构更新说明
│
├── tools/                         # 工具脚本
│   ├── generate_proto_desc.ps1    # Windows proto 生成脚本 ⭐ 新增
│   └── generate_proto_desc.sh     # Linux proto 生成脚本 ⭐ 新增
│
├── src/                           # C++ 服务器源代码
│   ├── battle/
│   ├── central/
│   ├── common/
│   ├── db/
│   ├── file/
│   ├── gateway/
│   ├── logic/
│   ├── login/
│   └── matching/
│
└── config/                        # 配置文件
    └── server_config/
```

## 设计原则

### 1. 职责分离
- **C++ 服务器**：使用 `protobuf/cpp/` 和 gRPC
- **Skynet 服务器**：使用 `protobuf/skynet/` 和 lua-protobuf

### 2. 生成文件统一放在 src/ 目录
- `protobuf/cpp/src/` - C++ 生成的代码
- `protobuf/skynet/src/` - Skynet 生成的 descriptor

### 3. 文档集中管理
- 所有 Skynet 相关文档放在 `docunment/skynet/`
- 保持与项目现有文档结构一致

## 文件说明

### Protobuf 相关

| 路径 | 说明 | 生成方式 |
|------|------|----------|
| `protobuf/cpp/*.proto` | C++ 服务器协议 | 手动编写 |
| `protobuf/cpp/src/*.pb.*` | C++ 生成代码 | CMake + protoc |
| `protobuf/skynet/*.proto` | Skynet 服务器协议 | 手动编写 |
| `protobuf/skynet/src/*.pb` | Skynet descriptor | `generate_proto_desc.ps1` |

### Skynet 服务

| 路径 | 说明 |
|------|------|
| `skynet_src/lualib/proto.lua` | Protobuf 编解码辅助库 |
| `skynet_src/service/logic/main.lua` | 逻辑服务器（玩家数据、业务逻辑） |
| `skynet_src/service/battle/main.lua` | 战斗服务器（战斗房间、帧同步） |
| `skynet_src/test_proto.lua` | Proto 功能测试脚本 |

### 文档

| 路径 | 说明 |
|------|------|
| `docunment/skynet/QUICKSTART.md` | 快速开始，适合初次使用 |
| `docunment/skynet/README_proto.md` | 详细文档，包含 API 和 FAQ |
| `docunment/skynet/INTEGRATION_SUMMARY.md` | 集成总结和后续步骤 |
| `protobuf/skynet/README.md` | Proto 文件使用说明 |

## 工作流程

### 开发 Skynet 服务

1. **定义协议**
   ```bash
   # 在 protobuf/skynet/ 下创建 .proto 文件
   vim protobuf/skynet/my_service.proto
   ```

2. **生成 Descriptor**
   ```powershell
   .\tools\generate_proto_desc.ps1
   # 生成到 protobuf/skynet/src/my_service.pb
   ```

3. **编写服务**
   ```lua
   -- skynet_src/service/my_service/main.lua
   local proto = require "proto"
   proto.load_descriptor("protobuf/skynet/src/my_service.pb")
   ```

4. **测试**
   ```bash
   cd skynet_src
   lua test_proto.lua
   ```

### 与 C++ 服务器通信

如果 Skynet 需要与 C++ 服务器通信：

1. **共享协议定义**
   - 在 `protobuf/skynet/` 下创建相同的 `.proto` 文件
   - 或者直接引用 `protobuf/cpp/` 中的定义（通过符号链接或复制）

2. **生成对应格式**
   - C++: 使用 CMake 生成 `.pb.cc/.pb.h`
   - Skynet: 使用 `generate_proto_desc.ps1` 生成 `.pb` descriptor

3. **实现通信层**
   - 使用 socket 连接
   - 统一消息格式（如：4字节长度 + protobuf 消息体）

## 快速参考

### 常用命令

```powershell
# 生成所有 Skynet proto descriptor
.\tools\generate_proto_desc.ps1

# 测试 proto 集成
cd skynet_src
lua test_proto.lua

# 查看生成的文件
ls protobuf\skynet\src\*.pb
```

### 常用路径

```lua
-- 在 Lua 代码中加载 descriptor
proto.load_descriptor("protobuf/skynet/src/server_logic.pb")
proto.load_descriptor("protobuf/skynet/src/server_battle.pb")
proto.load_descriptor("protobuf/skynet/src/common.pb")
```

## 注意事项

1. ✅ **生成文件不要手动修改**：`protobuf/*/src/` 下的文件由工具生成
2. ✅ **文档统一管理**：新增文档放在 `docunment/skynet/`
3. ✅ **遵循现有规范**：参考 `protobuf/cpp/` 的组织方式
4. ✅ **定期同步协议**：确保 C++ 和 Skynet 使用相同版本的 proto 定义

## 相关文档

- **Skynet 快速开始**: `docunment/skynet/QUICKSTART.md`
- **Proto 详细文档**: `docunment/skynet/README_proto.md`
- **Proto 文件说明**: `protobuf/skynet/README.md`
- **C++ 项目文档**: `docunment/server/`

---

**版本**: 1.0  
**更新日期**: 2025年11月22日  
**维护**: Skynet 集成团队
