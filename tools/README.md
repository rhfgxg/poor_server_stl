# Tools/Debug 脚本说明

本目录包含用于开发调试的辅助脚本。

## 📜 脚本列表

### WSL2 环境配置（新增）

#### `fix_line_endings.ps1`
修复 Shell 脚本的换行符问题

**用途**：
- 将 Shell 脚本从 Windows 换行符（CRLF）转换为 Unix 换行符（LF）
- 解决 WSL2 中运行脚本时的 `$'\r': command not found` 错误

**使用方法**：# 在 Windows PowerShell 中执行
.\tools\debug\fix_line_endings.ps1
---

#### `setup_wsl2_environment.sh`
WSL2 环境自动安装脚本

**用途**：
- 一键安装 WSL2 开发环境所需的所有工具和依赖
- 包括：GCC, CMake, Ninja, Git, Protobuf, MySQL, Redis, Lua, vcpkg 等

**使用方法**：# 在 WSL2 中执行
sudo bash tools/debug/setup_wsl2_environment.sh
**安装内容**：
- 基础开发工具
- Protocol Buffers 和 gRPC
- MySQL 客户端和开发库
- Redis 服务器
- Lua 环境
- vcpkg 包管理器
- 项目第三方库（可选）

**预计时间**：30-60 分钟

---

#### `check_environment.sh`
WSL2 环境检查脚本

**用途**：
- 快速检查所有开发工具是否已安装
- 显示工具版本信息
- 提供下一步建议

**使用方法**：bash tools/debug/check_environment.sh
---

#### `uninstall_wsl2_environment.sh`
WSL2 环境卸载脚本

**用途**：
- 清理环境（需要重新安装时使用）
- 询问式删除各个组件

**使用方法**：sudo bash tools/debug/uninstall_wsl2_environment.sh
---

### 配置管理

#### `copy_config.ps1`
复制配置文件到生成目录（用于 debug 调试）

**用途**：
- 将配置文件从 `config/cpp/cfg_server/` 复制到各服务器的生成目录
- 修改配置后需要执行此脚本

**使用方法**：.\tools\debug\copy_config.ps1
**复制的文件**：
- `cfg_battle_server.lua` → `out/build/x64-debug/src/battle/config/`
- `cfg_central_server.lua` → `out/build/x64-debug/src/central/config/`
- `cfg_db_server.lua` → `out/build/x64-debug/src/db/config/`
- `cfg_file_server.lua` → `out/build/x64-debug/src/file/config/`
- `cfg_gateway_server.lua` → `out/build/x64-debug/src/gateway/config/`
- `cfg_logic_server.lua` → `out/build/x64-debug/src/logic/config/`
- `cfg_login_server.lua` → `out/build/x64-debug/src/login/config/`
- `cfg_matching_server.lua` → `out/build/x64-debug/src/matching/config/`
- `cfg_db.lua` → `out/build/x64-debug/src/db/config/`（额外）

---

### Protobuf 代码生成

#### `proto_make_cpp.ps1` / `proto_make_cpp.sh`
生成 C++ Protobuf 和 gRPC 代码

**用途**：
- 从 `protobuf/cpp/*.proto` 生成 C++ 源码
- 生成文件保存到 `protobuf/cpp/src/`

**使用方法**：# Windows
.\tools\debug\proto_make_cpp.ps1

# Linux/macOS
bash tools/debug/proto_make_cpp.sh
**生成的文件**：
- `*.pb.cc` - Protobuf 实现
- `*.pb.h` - Protobuf 头文件
- `*.grpc.pb.cc` - gRPC 实现
- `*.grpc.pb.h` - gRPC 头文件

---

#### `proto_make_lua.ps1` / `proto_make_lua.sh`
生成 Skynet Protobuf descriptor 文件

**用途**：
- 从 `protobuf/skynet/*.proto` 生成 descriptor
- 生成文件保存到 `protobuf/skynet/src/`

**使用方法**：# Windows
.\tools\debug\proto_make_lua.ps1

# Linux/macOS  
bash tools/debug/proto_make_lua.sh
**生成的文件**：
- `*.pb` - Protobuf descriptor（二进制文件，供 lua-protobuf 使用）

**注意**：也可以使用项目根目录的 `tools/generate_proto_desc.ps1`

---

## 📋 开发流程

### WSL2 环境首次设置
# 1. 在 WSL2 中进入项目目录
cd /mnt/d/Project/cpp/poor/poor_server_stl

# 2. 添加执行权限
chmod +x tools/debug/*.sh

# 3. 检查当前环境
bash tools/debug/check_environment.sh

# 4. 运行自动安装脚本
sudo bash tools/debug/setup_wsl2_environment.sh

# 5. 重新加载环境变量
source ~/.bashrc

# 6. 验证安装
bash tools/debug/check_environment.sh

# 7. 编译 Proto 文件
bash tools/debug/proto_make_cpp.sh
bash tools/debug/proto_make_lua.sh

# 8. 编译项目
bash build.sh Release
### Windows 环境开发流程
# 1. 安装依赖（vcpkg）
vcpkg install

# 2. 生成 C++ proto 代码
.\tools\debug\proto_make_cpp.ps1

# 3. 生成 Lua proto descriptor
.\tools\debug\proto_make_lua.ps1

# 4. 复制配置文件
.\tools\debug\copy_config.ps1
### 修改 Proto 定义后
# Windows
.\tools\debug\proto_make_cpp.ps1
.\tools\debug\proto_make_lua.ps1

# Linux/WSL2
bash tools/debug/proto_make_cpp.sh
bash tools/debug/proto_make_lua.sh
### 修改配置文件后
# Windows
.\tools\debug\copy_config.ps1
---

## 🔧 跨平台支持

| 脚本功能 | Windows | Linux/macOS |
|---------|---------|-------------|
| 环境安装 | （手动）| `setup_wsl2_environment.sh` |
| 环境检查 | （手动）| `check_environment.sh` |
| 环境卸载 | （手动）| `uninstall_wsl2_environment.sh` |
| 复制配置 | `copy_config.ps1` | （暂无）|
| C++ Proto | `proto_make_cpp.ps1` | `proto_make_cpp.sh` |
| Skynet Proto | `proto_make_lua.ps1` | `proto_make_lua.sh` |
| 修复换行符 | `fix_line_endings.ps1` | （不需要）|

---

## ⚙️ 配置

### 路径配置
所有脚本自动检测项目路径，无需手动配置。

### Protoc 工具
脚本会自动查找：
1. 系统 PATH 中的 protoc
2. vcpkg 安装的 protoc（`vcpkg_installed/x64-windows/tools/protobuf/protoc.exe`）
3. vcpkg 安装的 protoc（`vcpkg_installed/x64-linux/tools/protobuf/protoc`）

---

## 📝 注意事项

1. **Windows 执行策略**：首次运行可能需要设置 PowerShell 执行策略Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
2. **WSL2 换行符问题**：如果在 WSL2 中遇到 `$'\r': command not found` 错误# 在 Windows 中运行
.\tools\debug\fix_line_endings.ps1

# 然后在 WSL2 中重新添加执行权限
chmod +x tools/debug/*.sh
3. **配置文件路径**：确保 `config/cpp/cfg_server/` 目录存在

4. **生成目录**：确保已编译项目，`out/build/x64-debug/` 目录存在

5. **Proto 文件**：修改 proto 文件后必须重新生成代码

---

## 🆘 常见问题

### Q: WSL2 中提示 `$'\r': command not found`
**A:** 这是换行符问题，执行：# 在 Windows PowerShell 中
.\tools\debug\fix_line_endings.ps1

# 然后在 WSL2 中
chmod +x tools/debug/*.sh
### Q: 提示 "protoc not found"
**A:** 确保已通过 vcpkg 安装 protobuf：# WSL2
~/vcpkg/vcpkg install protobuf grpc

# Windows
vcpkg install protobuf grpc
### Q: 复制配置失败
**A:** 检查：
1. `config/cpp/cfg_server/` 目录是否存在
2. 是否已编译项目（生成目录存在）

### Q: Proto 生成失败
**A:** 检查：
1. `.proto` 文件语法是否正确
2. 是否有循环依赖

### Q: WSL2 系统包配置错误
**A:** 这些错误不影响项目编译，可以忽略。如需修复：sudo apt-get install -f
sudo dpkg --configure -a
---

## 📚 相关文档

- **WSL2 环境配置指南**：`docunment/项目配置与运行/WSL2环境配置指南.md`
- **WSL2 快速启动**：`docunment/项目配置与运行/WSL2快速启动.md`
- **WSL2 文件总结**：`docunment/项目配置与运行/WSL2文件总结.md`

---

**更新日期**: 2025年11月23日
