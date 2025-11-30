# Poor Server STL

> 🎮 基于 C++ 的分布式游戏服务器框架，集成 Skynet + Protobuf，支持炉石传说卡牌游戏和网盘功能

[![C++20](https://img.shields.io/badge/C++-20-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.8+-064F8C?style=flat&logo=cmake)](https://cmake.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)](https://github.com/rhfgxg/poor_server_stl)

---

## 📚 目录

- [项目简介](#项目简介)
- [核心特性](#核心特性)
- [快速开始](#快速开始)
- [项目架构](#项目架构)
- [开发环境](#开发环境)
- [项目文档](#项目文档)
- [参与贡献](#参与贡献)
- [许可证](#许可证)

---

## 🎯 项目简介

Poor Server STL 是一个现代化的 C++ 游戏服务器框架，旨在学习和实践分布式系统、微服务架构、高性能网络编程等技术。

### 项目目标

#### 1. 架构设计与技术实践
- ✅ **分布式架构** - 主从服务器，微服务化设计
- ✅ **RPC 通信** - 基于 gRPC 的服务间通信
- ✅ **Protobuf 协议** - 高效的数据序列化
- ✅ **多线程与异步** - 高并发处理能力
- ✅ **日志系统** - 完善的日志记录和追踪
- ✅ **Lua 集成** - 配置热更新，逻辑脚本化
- ✅ **Skynet 框架** - 逻辑服务器和战斗服务器

#### 2. 功能实现
- 🎮 **炉石传说** - 卡牌游戏核心玩法
- 📁 **网盘系统** - 文件上传下载、断点续传、图片预览压缩

#### 3. 性能优化
- ⚡ **连接池** - 服务器和数据库连接池
- 🔄 **对象池** - 减少对象创建销毁开销
- 💾 **多级缓存** - 内存 → Redis → 数据库

---

## ✨ 核心特性

### 🏗️ 分布式架构
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Gateway   │────▶│   Central   │────▶│   Database  │
│   网关服务器 │     │   中心服务器 │     │   数据库服务 │
└─────────────┘     └─────────────┘     └─────────────┘
       │                   │                    │
       │            ┌──────┴──────┐            │
       │            │             │            │
       ▼            ▼             ▼            ▼
┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐
│  Login   │  │  Logic   │  │  Battle  │  │   File   │
│登录服务器 │  │逻辑服务器 │  │战斗服务器 │  │文件服务器 │
└──────────┘  └──────────┘  └──────────┘  └──────────┘
### 🚀 技术栈

| 技术 | 说明 |
|------|------|
| **C++20** | 现代 C++ 特性 |
| **CMake + Ninja** | 跨平台构建系统 |
| **gRPC + Protobuf** | RPC 框架和序列化 |
| **Skynet** | Lua 游戏服务器框架 |
| **MySQL** | 关系型数据库 |
| **Redis** | 缓存和会话存储 |
| **vcpkg** | C++ 包管理器 |
| **Docker** | 容器化部署 |

---

## 🚀 快速开始

### ⚠️ 重要：首次设置代码规范（必须）

克隆项目后，**请先完成以下步骤**以确保代码格式统一：

```powershell
# Windows
cd poor_server_stl

# 1. 统一现有文件编码和换行符（首次必须运行）
.\tools\debug\windows\fix_encoding_and_lineendings.ps1

# 2. 安装 Git 提交前检查（强烈推荐）
.\tools\debug\windows\install_git_hooks.ps1
```

```bash
# Linux / WSL
cd poor_server_stl

# 1. 统一现有文件编码和换行符（首次必须运行）
bash tools/debug/linux/fix_encoding_and_lineendings.sh

# 2. 安装 Git 提交前检查（强烈推荐）
bash tools/debug/linux/install_git_hooks.sh
```

**为什么需要这一步？**
- ✅ 统一所有文件为 UTF-8 无 BOM 编码
- ✅ 统一换行符为 LF（Linux 标准）
- ✅ 防止提交不规范的代码到仓库
- ✅ 避免跨平台协作时的编码冲突

**详细说明**: [编码规范快速指南](docunment/layout/ENCODING_QUICKSTART.md)

---

### 方式 1: Docker 快速部署（推荐）⭐

**耗时：15 分钟**
# 1. 克隆项目
git clone https://github.com/rhfgxg/poor_server_stl.git
cd poor_server_stl

# 2. 启动所有服务（MySQL, Redis, 游戏服务器等）
docker compose up -d

# 3. 查看服务状态
docker compose ps

# ✅ 完成！所有服务已启动
**详细文档**: [Docker 快速部署指南](docunment/项目配置与运行/docker/README.md)

---

### 方式 2: 传统开发环境

#### Windows (Visual Studio 2022)
# 1. 克隆项目
git clone https://github.com/rhfgxg/poor_server_stl.git
cd poor_server_stl

# 2. 安装依赖
vcpkg install

# 3. 生成 Protobuf 文件
.\tools\debug\protoc_make.cmd
.\tools\generate_proto_desc.ps1

# 4. 编译项目
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release

# 5. 复制配置文件
.\tools\debug\copy_config.cmd
#### Linux / WSL2
# 1. 克隆项目
git clone https://github.com/rhfgxg/poor_server_stl.git
cd poor_server_stl

# 2. 安装依赖
sudo apt install build-essential cmake ninja-build
~/vcpkg/vcpkg install

# 3. 生成 Protobuf 文件
bash tools/debug/wsl/proto_make_cpp.sh
bash tools/generate_proto_desc.sh

# 4. 编译项目（推荐使用 Release 模式）
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
ninja -C build

# 5. 复制配置文件
bash tools/debug/wsl/copy_config.sh

# 或使用交互式编译指南（推荐）
bash tools/debug/wsl/compile_guide.sh
**详细文档**: 
- [Windows 环境配置](docunment/项目配置与运行/windows开发工具与环境配置.md)
- [Linux/WSL2 环境配置](docunment/项目配置与运行/linux开发工具与环境配置.md)

---

## 🏗️ 项目架构

### 服务器模块

| 服务器 | 端口 | 功能 | 技术栈 |
|--------|------|------|--------|
| **Central** | 50051 | 中心服务器，服务注册与发现 | C++ + gRPC |
| **Gateway** | 50054 | 网关，客户端连接入口 | C++ + gRPC |
| **Login** | 50053 | 登录认证，JWT Token | C++ + gRPC |
| **DB** | 50052 | 数据库代理，数据持久化 | C++ + MySQL |
| **Logic** | - | 游戏逻辑服务器 | Skynet + Lua |
| **Battle** | - | 战斗服务器，帧同步 | Skynet + Lua |
| **File** | 50055 | 文件上传下载，网盘功能 | C++ + gRPC |
| **Matching** | 50056 | 匹配服务器，玩家匹配 | C++ + gRPC |

### 数据库设计

- **poor_users** - 用户账号、登录、权限
- **hearthstone** - 炉石传说游戏数据
- **poor_file** - 网盘文件存储

**详细文档**: [数据库设计](docunment/server/数据库/数据库总览.md)

### 目录结构
poor_server_stl/
├── src/                    # C++ 服务器源码
│   ├── central/            # 中心服务器
│   ├── gateway/            # 网关服务器
│   ├── login/              # 登录服务器
│   ├── db/                 # 数据库服务器
│   ├── file/               # 文件服务器
│   ├── matching/           # 匹配服务器
│   └── common/             # 公共库
│
├── skynet_src/             # Skynet 服务器源码
│   ├── lualib/             # Lua 库
│   │   └── proto.lua       # Protobuf 辅助库
│   └── service/            # Skynet 服务
│       ├── logic/          # 逻辑服务器
│       └── battle/         # 战斗服务器
│
├── protobuf/               # Protobuf 协议定义
│   ├── cpp/                # C++ gRPC 协议
│   │   ├── *.proto
│   │   └── src/            # 生成的代码
│   └── skynet/             # Skynet 协议
│       ├── *.proto
│       └── src/            # 生成的 descriptor
│
├── config/                 # 配置文件
│   └── server_config.lua   # 服务器配置
│
├── docunment/              # 项目文档
│   ├── README.md           # 文档中心
│   ├── server/             # 服务器架构文档
│   ├── skynet/             # Skynet 学习文档
│   └── 项目配置与运行/      # 环境配置文档
│       └── docker/         # Docker 部署文档
│
├── tools/                  # 工具脚本
│   ├── generate_proto_desc.ps1  # Proto descriptor 生成
│   └── debug/              # 调试脚本
│
├── vcpkg.json              # 依赖包清单
└── CMakeLists.txt          # CMake 构建配置
---

## 🛠️ 开发环境

### 系统要求

| 项目 | 最低要求 | 推荐配置 |
|------|---------|---------|
| **操作系统** | Windows 10 / Ubuntu 20.04 | Windows 11 / Ubuntu 22.04 |
| **CPU** | 4 核 | 8 核+ |
| **内存** | 8 GB | 16 GB+ |
| **磁盘** | 20 GB | 50 GB+ SSD |

### 开发工具

#### Windows
- **Visual Studio 2022** - IDE 和编译器
- **vcpkg** - C++ 包管理器
- **Docker Desktop** - 容器化（可选）
- **MySQL 8.0+** - 数据库
- **Redis 7.0+** - 缓存

#### Linux
- **GCC 11+ / Clang 13+** - 编译器
- **CMake 3.8+** - 构建工具
- **Ninja** - 构建系统
- **vcpkg** - 包管理器
- **Docker** - 容器化（可选）

### 第三方库

完整依赖列表见 [vcpkg.json](vcpkg.json)

**核心库：**
- gRPC + Protobuf - RPC 框架
- MySQL Connector C++ - 数据库驱动
- cpp_redis - Redis 客户端
- nlohmann/json - JSON 处理
- spdlog - 日志库
- jwt-cpp - JWT 认证
- OpenSSL - 加密库
- Boost - C++ 工具库

**详细文档**: [第三方库说明](docunment/server/library.md)

---

## 📖 项目文档

### 📚 完整文档导航

**[→ 文档中心](docunment/README.md)** - 所有文档的总索引（推荐从这里开始）

### 快速链接

#### 🚀 快速开始
- [环境配置总览](docunment/项目配置与运行/README.md) - 选择适合你的配置方式
- [Docker 快速部署](docunment/项目配置与运行/docker/README.md) - 15 分钟快速开始
- [编译运行指南](docunment/项目配置与运行/编译及运行项目.md) - 传统方式编译

#### 🏗️ 架构设计
- [服务器架构](docunment/server/readme.md) - 分布式架构设计
- [服务器模型](docunment/server/model.md) - 各服务器职责
- [数据库设计](docunment/server/数据库/数据库总览.md) - 多数据库架构

#### 🌙 Skynet 学习
- [Skynet 文档导航](docunment/skynet/README_skynet.md) - Skynet 学习路线
- [Skynet 快速入门](docunment/skynet/skynet_quickstart.md) - 2-3 小时入门
- [Skynet Protobuf 集成](docunment/skynet/QUICKSTART.md) - Protobuf 使用

#### 🐋 Docker 专题
- [Docker 环境配置](docunment/项目配置与运行/docker/Docker_环境配置指南.md) - Docker Desktop 安装
- [Docker 能保存什么](docunment/项目配置与运行/docker/Docker_能保存什么.md) - 理解 Docker 原理
- [Docker 部署指南](docunment/项目配置与运行/docker/Docker_部署指南.md) - 实战部署
- [Docker 快速参考](docunment/项目配置与运行/docker/Docker_快速参考.md) - 命令速查

#### 📝 开发规范
- [C++ 代码规范](docunment/layout/cpp_layout.md) - 命名、格式、注释
- [C++ 安全指南](docunment/layout/cpp_security.md) - 安全最佳实践
- [编码格式规范](docunment/layout/layout.md) - 文件编码要求

---

## 🤝 参与贡献

欢迎任何形式的贡献！

**→ [完整贡献指南 (中文)](CONTRIBUTING_ZH.md) | [Contributing Guide (English)](CONTRIBUTING.md)**

### 📋 贡献前准备（必读）

**首次贡献者请务必完成以下设置：**

1. **安装代码规范工具**（自动化检查）
   ```powershell
   # Windows
   .\tools\debug\windows\install_git_hooks.ps1
   
   # Linux/WSL
   bash tools/debug/linux/install_git_hooks.sh
   ```

2. **修复现有文件格式**（如果有编码问题）
   ```powershell
   # Windows
   .\tools\debug\windows\fix_encoding_and_lineendings.ps1
   
   # Linux/WSL
   bash tools/debug/linux/fix_encoding_and_lineendings.sh
   ```

**详细指南**: 
- [完整贡献指南 (CONTRIBUTING.md)](CONTRIBUTING.md)
- [编码规范文档](docunment/layout/ENCODING_QUICKSTART.md)

---

### 贡献流程

1. **Fork 项目** - 点击右上角 Fork 按钮
2. **克隆到本地** - `git clone https://github.com/YOUR_USERNAME/poor_server_stl.git`
3. **设置代码规范** - 运行上述准备步骤
4. **创建分支** - `git checkout -b feature/amazing-feature`
5. **开发代码** - 遵循项目规范
6. **提交代码** - `git commit -m 'feat: add some amazing feature'`
7. **推送分支** - `git push origin feature/amazing-feature`
8. **发起 PR** - 在 GitHub 上发起 Pull Request

### 代码规范检查清单 ✅

提交前请确认：

- [ ] 所有文件使用 **UTF-8 无 BOM** 编码
- [ ] 所有文件使用 **LF** 换行符（Windows 脚本除外）
- [ ] 代码遵循 [C++ 代码规范](docunment/layout/cpp_layout.md)
- [ ] 添加了必要的注释和文档
- [ ] 代码通过编译，无警告
- [ ] 提交信息清晰（使用 [约定式提交](https://www.conventionalcommits.org/zh-hans/)）

**提示**: 如果安装了 Git hook，不符合规范的代码会被自动拒绝提交！

---

### 开发规范

- ✅ 遵循 [C++ 代码规范](docunment/layout/cpp_layout.md)
- ✅ 遵循 [C++ 安全指南](docunment/layout/cpp_security.md)
- ✅ 遵循 [文件编码规范](docunment/layout/layout.md)
- ✅ 添加必要的注释和文档
- ✅ 确保代码通过编译
- ✅ 编写单元测试（如适用）
- ✅ 提交信息清晰明确

### 提交信息规范

使用 [约定式提交](https://www.conventionalcommits.org/zh-hans/) 格式：

```
<type>(<scope>): <subject>

<body>

<footer>
```

**类型 (type)**:
- `feat`: 新功能
- `fix`: 修复 Bug
- `docs`: 文档更新
- `style`: 代码格式（不影响功能）
- `refactor`: 重构
- `perf`: 性能优化
- `test`: 测试相关
- `chore`: 构建/工具相关

**示例**:
```
feat(gateway): add connection pool for client connections

- Implement connection pool with configurable size
- Add connection timeout handling
- Update documentation

Closes #123
```

### 需要帮助的领域

- 📝 完善文档和示例
- 🐛 修复 Bug
- ✨ 实现新功能
- 🎨 优化代码和架构
- 🧪 编写测试用例
- 🌍 多语言支持
- 🔧 改进开发工具
