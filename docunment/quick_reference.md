# Poor Server STL - 快速命令参考

> **最后更新**: 2025年1月  
> **版本**: v2.0

---

## 🚀 一键编译和启动

### 完整流程（首次使用）

```bash
# 1. 编译所有内容（包括 Skynet）
bash tools/debug/wsl/compile_guide.sh

# 2. 启动所有服务
bash tools/debug/wsl/manage_services.sh start

# 3. 查看状态
bash tools/debug/wsl/manage_services.sh status
```

### 日常开发流程

```bash
# 修改代码后重新编译
cd build
ninja

# 重启服务
bash tools/debug/wsl/manage_services.sh restart

# 查看状态
bash tools/debug/wsl/manage_services.sh status
```

---

## 📦 编译相关

### 完整编译指南

```bash
# 交互式编译指南（推荐新手）
bash tools/debug/wsl/compile_guide.sh
```

### 手动编译步骤

```bash
# 1. 编译 Skynet
cd skynet_src/skynet
make linux

# 2. 生成 Protobuf 代码
bash tools/debug/wsl/proto_make_cpp.sh
bash tools/debug/wsl/proto_make_lua.sh

# 3. 编译 C++ 项目
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake ..
ninja

# 4. 编译特定模块
ninja gateway
ninja central
ninja db
```

### 清理和重新编译

```bash
# 清理 build 目录
cd build
rm -rf *

# 重新配置和编译
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake ..
ninja
```

---

## 🎮 服务管理

### 基本命令

```bash
# 查看所有可用命令
bash tools/debug/wsl/manage_services.sh help

# 启动所有服务（数据库 + Skynet + C++ 服务器）
bash tools/debug/wsl/manage_services.sh start

# 停止所有服务
bash tools/debug/wsl/manage_services.sh stop

# 重启所有服务
bash tools/debug/wsl/manage_services.sh restart

# 查看运行状态
bash tools/debug/wsl/manage_services.sh status
```

### 分步启动

```bash
# 1. 仅启动数据库（Redis + MySQL）
bash tools/debug/wsl/manage_services.sh start-db

# 2. 仅启动 Skynet
bash tools/debug/wsl/manage_services.sh start-skynet

# 3. 手动启动 C++ 服务器
cd build/src/central && ./central &
cd build/src/gateway && ./gateway &
cd build/src/db && ./db &
```

### 分步停止

```bash
# 仅停止 Skynet
bash tools/debug/wsl/manage_services.sh stop-skynet

# 仅停止数据库
bash tools/debug/wsl/manage_services.sh stop-db

# 停止所有
bash tools/debug/wsl/manage_services.sh stop
```

---

## 🔧 配置管理

### 复制配置文件

```bash
# 自动复制所有配置文件
bash tools/debug/wsl/manage_services.sh copy-config

# 或使用专门的脚本
bash tools/debug/wsl/copy_config.sh
```

### 配置文件位置

```
config/
├── cpp/
│   └── cfg_server/
│       ├── cfg_central.lua
│       ├── cfg_gateway.lua
│       ├── cfg_db.lua
│       ├── cfg_login.lua
│       └── ...
└── skynet/
    └── config.lua
```

---

## 🗄️ 数据库管理

### 启动/停止数据库

```bash
# 启动 Redis + MySQL
bash tools/debug/wsl/manage_services.sh start-db

# 停止数据库
bash tools/debug/wsl/manage_services.sh stop-db

# 检查 Redis
redis-cli ping

# 检查 MySQL（WSL）
sudo service mysql status

# 连接到 MySQL
mysql -h 127.0.0.1 -u root -p
```

### 创建数据库

```sql
-- 创建数据库
CREATE DATABASE poor_hearthstone CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 使用数据库
USE poor_hearthstone;

-- 导入表结构
SOURCE docunment/server/数据库/sql定义文件/xxx.sql;
```

---

## 🐛 调试和日志

### 查看进程状态

```bash
# 查看所有服务状态
bash tools/debug/wsl/manage_services.sh status

# 查看具体进程
ps aux | grep central
ps aux | grep gateway
ps aux | grep skynet

# 查看端口占用
netstat -an | grep 8888   # Skynet
netstat -an | grep 50053  # Gateway
netstat -an | grep 50051  # Central
```

### 查看日志

```bash
# 如果配置了日志文件
tail -f logs/central.log
tail -f logs/gateway.log
tail -f logs/skynet.log

# 查看所有日志
tail -f logs/*.log

# 实时查看终端输出（前台运行）
cd build/src/gateway && ./gateway
```

### 调试模式运行

```bash
# 使用 gdb
cd build/src/gateway
gdb ./gateway

# gdb 命令
(gdb) run
(gdb) bt        # 查看调用栈
(gdb) quit
```

---

## 📊 性能监控

### 检查资源使用

```bash
# CPU 和内存
top -p $(pgrep -d',' -x 'central|gateway|db|skynet')

# 或使用 htop（更友好）
htop

# 网络连接
netstat -an | grep ESTABLISHED
```

### Skynet 调试控制台

```bash
# 连接到 Skynet 调试控制台（如果配置了）
telnet 127.0.0.1 8000

# 常用命令
> list          # 列出所有服务
> info :地址    # 查看服务信息
> kill :地址    # 杀死服务
> reload 模块   # 热更新模块
```

---

## 🔄 开发工作流

### 修改 Lua 代码（Skynet）

```bash
# 1. 修改代码
vim skynet_src/service/logic/player_agent.lua

# 2a. 热更新（无需重启，推荐）
telnet 127.0.0.1 8000
> reload("logic.player_agent")

# 2b. 或重启 Skynet
bash tools/debug/wsl/manage_services.sh stop-skynet
bash tools/debug/wsl/manage_services.sh start-skynet
```

### 修改 C++ 代码

```bash
# 1. 修改代码
vim src/gateway/server/gateway_server.cpp

# 2. 重新编译
cd build
ninja gateway

# 3. 重启服务
pkill gateway
cd src/gateway && ./gateway &

# 或使用脚本重启所有
bash tools/debug/wsl/manage_services.sh restart
```

### 修改 Protobuf

```bash
# 1. 修改 .proto 文件
vim protobuf/cpp/server_gateway.proto

# 2. 重新生成代码
bash tools/debug/wsl/proto_make_cpp.sh

# 3. 重新编译
cd build
ninja

# 4. 重启服务
bash tools/debug/wsl/manage_services.sh restart
```

---

## 🧪 测试

### 单元测试（如果有）

```bash
cd build
ctest

# 或运行特定测试
./tests/test_gateway
```

### 集成测试

```bash
# 使用 grpcurl 测试 gRPC 接口
grpcurl -plaintext localhost:50053 list
grpcurl -plaintext -d '{"username":"test"}' localhost:50053 rpc_server.GatewayServer/SomeMethod
```

---

## 📚 常用路径

```
项目根目录:     /mnt/d/Project/cpp/poor/poor_server_stl/
构建目录:       build/
Skynet 目录:    skynet_src/skynet/
配置目录:       config/
日志目录:       logs/
文档目录:       docunment/
工具脚本:       tools/debug/wsl/
```

---

## ⚡ 常见问题快速解决

### Redis 未运行

```bash
sudo service redis-server start
```

### MySQL 连接失败

```bash
# WSL MySQL
sudo service mysql start

# Windows MySQL
# 在 Windows 中启动 MySQL 服务
```

### Skynet 启动失败

```bash
# 方法 1: 一键修复（推荐）
bash tools/skynet/fix_skynet.sh

# 方法 2: 诊断问题
bash tools/skynet/diagnose_skynet.sh

# 方法 3: 查看日志
tail -50 logs/skynet.log

# 方法 4: 重新完整编译
bash tools/skynet/build_skynet.sh

# 方法 5: 前台启动查看错误
cd skynet_src/skynet
./skynet ../../config/skynet/config.lua

# 常见问题:
# - C 服务库缺失: 需要完整编译 (make cleanall && make linux)
# - 路径配置错误: 检查 config/skynet/config.lua
# - 服务文件缺失: 检查 skynet_src/service/main.lua

# 详细排查请参考: 
# - docunment/skynet/troubleshooting_skynet.md
# - docunment/skynet/path_fix.md
```

### 端口被占用

```bash
# 查找占用端口的进程
lsof -i :8888
lsof -i :50053

# 杀死进程
kill -9 <PID>
```

### 服务启动失败

```bash
# 查看详细状态
bash tools/debug/wsl/manage_services.sh status

# 查看日志（如果配置了）
tail -f logs/*.log

# 前台运行查看错误
cd build/src/gateway && ./gateway
```

---

## 🎯 完整示例：从头开始

```bash
# 1. 克隆项目（如果还没有）
git clone https://github.com/rhfgxg/poor_server_stl
cd poor_server_stl

# 2. 安装环境（首次）
bash tools/wsl_environment/setup_wsl2_environment.sh

# 3. 编译所有内容
bash tools/debug/wsl/compile_guide.sh

# 4. 启动服务
bash tools/debug/wsl/manage_services.sh start

# 5. 查看状态
bash tools/debug/wsl/manage_services.sh status

# 6. 测试（使用客户端或 grpcurl）

# 7. 停止服务
bash tools/debug/wsl/manage_services.sh stop
```

---

**提示**: 将此文件加入书签，方便随时查阅！
