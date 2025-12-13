# Tools 工具脚本说明

本目录包含项目所需的所有工具脚本，已完成整理优化。

---

## 📁 目录结构

```
tools/
├── setup/          # 环境安装与检查
├── service/        # 服务编译与管理（统一入口）
└── windows/        # Windows 脚本
```

---

## 🚀 快速开始

### 1. 首次安装（WSL2）

```bash
# 一键安装所有依赖（包括 Skynet + lua-protobuf）
sudo bash tools/setup/wsl_setup.sh

# 检查环境
bash tools/setup/wsl_check.sh
```

### 2. 编译项目

```bash
# 一键编译所有组件（Proto + Skynet + C++）
bash tools/service/manage.sh compile

# 或指定编译模式
bash tools/service/manage.sh compile Debug
```

### 3. 启动服务

```bash
# 一键启动所有服务（数据库 + Skynet + C++ 服务器）
bash tools/service/manage.sh start

# 查看运行状态
bash tools/service/manage.sh status

# 停止所有服务
bash tools/service/manage.sh stop
```

---

## 📂 核心脚本

### setup/ - 环境安装

| 脚本 | 说明 | 用途 |
|------|------|------|
| `wsl_setup.sh` | WSL2 环境一键安装 | 安装所有依赖 + 编译 Skynet + 安装 lua-protobuf |
| `wsl_check.sh` | 环境检查 | 验证工具是否安装正确 |
| `wsl_uninstall.sh` | 环境卸载 | 清理环境（重装时使用） |

**功能说明**：

`wsl_setup.sh` 自动完成：
- ✅ 安装基础开发工具（GCC、CMake、Ninja等）
- ✅ 安装 Protobuf 编译器
- ✅ 安装 MySQL 客户端和 Redis
- ✅ 安装 Lua 环境
- ✅ 安装 vcpkg 包管理器
- ✅ 编译 Skynet 框架
- ✅ 下载并编译 lua-protobuf
- ✅ 安装项目依赖库（可选）

---

### service/ - 服务管理（统一入口）

| 脚本 | 说明 | 功能 |
|------|------|------|
| `manage.sh` | 统一管理脚本 | 编译、启动、停止所有服务 |

**功能说明**：

`manage.sh` 提供以下命令：

```bash
# 编译相关
bash tools/service/manage.sh compile [Release|Debug]  # 完整编译（Proto + Skynet + C++）
bash tools/service/manage.sh proto                    # 仅生成 Protobuf 代码
bash tools/service/manage.sh skynet                   # 仅编译 Skynet
bash tools/service/manage.sh cpp [Release|Debug]      # 仅编译 C++ 项目

# 服务管理
bash tools/service/manage.sh start                    # 启动所有服务
bash tools/service/manage.sh stop                     # 停止所有服务
bash tools/service/manage.sh restart                  # 重启所有服务
bash tools/service/manage.sh status                   # 查看服务状态
```

**管理的服务**：
- Redis 数据库
- Skynet 逻辑服务器
- Central 服务器
- DB 服务器
- Login 服务器
- Gateway 服务器
- File 服务器
- Matching 服务器

---

### windows/ - Windows 脚本

| 脚本 | 说明 | 用途 |
|------|------|------|
| `proto_make_cpp.ps1` | 生成 C++ Protobuf 代码 | Windows 开发环境 |
| `proto_make_lua.ps1` | 生成 Lua Descriptor | Windows 开发环境 |
| `copy_config.ps1` | 复制配置文件 | 部署配置 |
| `fix_encoding.ps1` | 修复文件编码 | 代码规范 |
| `install_git_hooks.ps1` | 安装 Git Hooks | 代码检查 |

---

## 🔄 常用工作流

### 开发环境首次搭建

```bash
# 1. 安装依赖
sudo bash tools/setup/wsl_setup.sh

# 2. 编译项目
bash tools/service/manage.sh compile

# 3. 启动服务
bash tools/service/manage.sh start

# 4. 查看状态
bash tools/service/manage.sh status
```

### 修改代码后重新编译

```bash
# 重新编译
bash tools/service/manage.sh compile

# 重启服务
bash tools/service/manage.sh restart
```

### 日常开发

```bash
# 启动服务
bash tools/service/manage.sh start

# 查看状态
bash tools/service/manage.sh status

# 停止服务
bash tools/service/manage.sh stop
```

---

## ⚠️ 注意事项

1. **脚本权限**：首次使用需添加执行权限
   ```bash
   chmod +x tools/**/*.sh
   ```

2. **环境变量**：安装 vcpkg 后需重新加载
   ```bash
   source ~/.bashrc
   ```

3. **服务依赖**：确保 Redis 已启动
   ```bash
   sudo service redis-server start
   ```

4. **编译模式**：WSL 环境推荐使用 Release 模式
   ```bash
   bash tools/service/manage.sh compile Release
   ```

---

## 📚 相关文档

- **环境配置**: `docunment/项目配置与运行/WSL2环境配置指南.md`
- **快速启动**: `docunment/skynet/QUICK_START.md`
- **架构设计**: `docunment/architecture/final_architecture_v2.md`

---

## 🔧 故障排查

### 问题 1: 编译失败

```bash
# 检查环境
bash tools/setup/wsl_check.sh

# 查看详细错误
bash tools/service/manage.sh compile 2>&1 | tee compile.log
```

### 问题 2: 服务启动失败

```bash
# 查看状态
bash tools/service/manage.sh status

# 查看日志
tail -f logs/*.log
```

### 问题 3: Redis 未运行

```bash
# 启动 Redis
sudo service redis-server start

# 检查状态
redis-cli ping
```

---

## 📊 整理成果

### 脚本优化

| 指标 | 整理前 | 整理后 | 优化 |
|------|--------|--------|------|
| 脚本数量 | 22 个 | 8 个 | ↓ 63% |
| 核心功能脚本 | 10 个 | 2 个 | ↓ 80% |
| 用户需记住的命令 | 6+ 条 | 3 条 | ↓ 50% |

### 功能整合

**整理前**：
- 编译：3 个独立脚本
- Skynet：5 个独立脚本
- 服务管理：10 个独立脚本

**整理后**：
- 环境安装：1 个脚本（`wsl_setup.sh`）
- 服务管理：1 个脚本（`manage.sh`）

---

**更新时间**: 2025-12-13  
**维护者**: 开发团队
