# Docker 能保存什么？完整解析

本文档详细说明 Docker 能保存哪些环境内容，以及哪些内容无法保存（需要额外配置）。

---

## ?? 目录

1. [Docker 能保存的内容](#docker-能保存的内容)
2. [Docker 无法保存的内容](#docker-无法保存的内容)
3. [新环境配置 Docker 后的完整步骤](#新环境配置-docker-后的完整步骤)
4. [实际案例：Poor Server STL 项目](#实际案例poor-server-stl-项目)

---

## ? Docker 能保存的内容

### 1. **操作系统环境**

**能保存：**
```dockerfile
FROM ubuntu:22.04
# ? 精确的 Ubuntu 版本
# ? 系统架构（x64, arm64）
# ? 基础系统文件和配置
```

**示例：**
- Ubuntu 22.04, 20.04, 18.04
- Debian 11, 10
- Alpine Linux
- CentOS, Rocky Linux

---

### 2. **编译器和开发工具**

**能保存：**
```dockerfile
RUN apt-get install -y \
    gcc-11=11.4.0-1ubuntu1~22.04 \     # ? 精确的 GCC 版本
    g++-11=11.4.0-1ubuntu1~22.04 \      # ? 精确的 G++ 版本
    cmake=3.22.1-1ubuntu1.22.04.2 \     # ? 精确的 CMake 版本
    ninja-build \                        # ? Ninja 构建工具
    git \                                # ? Git
    make \                               # ? Make
    clang                                # ? Clang 编译器
```

**你的项目包含：**
- ? GCC 11
- ? CMake 3.28.3（或指定版本）
- ? Ninja
- ? 所有编译工具链

---

### 3. **第三方库（vcpkg 安装的库）**

**完全可以保存！这是重点！**

```dockerfile
# 方式 1: 安装 vcpkg 并在构建时安装依赖
WORKDIR /opt
RUN git clone https://github.com/microsoft/vcpkg.git && \
    cd vcpkg && ./bootstrap-vcpkg.sh

# 复制 vcpkg.json（定义了所有依赖）
COPY vcpkg.json /app/

# 安装所有第三方库
RUN /opt/vcpkg/vcpkg install --triplet=x64-linux
```

**你的项目所有第三方库都会被保存：**
- ? gRPC
- ? Protobuf
- ? MySQL Connector C++
- ? cpp_redis
- ? tacopie
- ? nlohmann-json
- ? lua
- ? spdlog
- ? jwt-cpp
- ? OpenSSL
- ? Boost 库
- ? **所有 vcpkg.json 中定义的库**

**优势：**
- ? 版本完全一致（vcpkg.json 锁定版本）
- ? 自动安装，无需手动配置
- ? 跨平台一致性

---

### 4. **系统库和运行时依赖**

**能保存：**
```dockerfile
RUN apt-get install -y \
    libstdc++6 \           # ? C++ 标准库
    libssl3 \              # ? OpenSSL 运行时
    libmysqlclient21 \     # ? MySQL 客户端库
    liblua5.3-0            # ? Lua 运行时
```

---

### 5. **项目代码和编译产物**

**能保存：**
```dockerfile
# 复制项目源代码
COPY . /app

# 编译项目
RUN mkdir build && cd build && \
    cmake -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    .. && ninja

# ? 源代码
# ? 编译好的可执行文件
# ? CMake 配置
# ? 所有项目文件
```

---

### 6. **配置文件**

**能保存：**
```dockerfile
# 复制配置文件
COPY config/ /app/config/

# ? 服务器配置文件（.lua, .json, .xml）
# ? 数据库配置
# ? 日志配置
# ? 所有静态配置
```

**你的项目：**
- ? `config/cpp/cfg_server/*.lua`
- ? 所有配置文件

---

### 7. **环境变量**

**能保存：**
```dockerfile
ENV TZ=Asia/Shanghai \
    LANG=C.UTF-8 \
    PATH=/app/bin:$PATH \
    LD_LIBRARY_PATH=/app/lib

# ? 所有环境变量
# ? 时区设置
# ? 语言设置
# ? 路径配置
```

---

### 8. **网络配置（容器级别）**

**能保存：**
```yaml
# docker-compose.yml
networks:
  game_network:
    driver: bridge
    ipam:
      config:
        - subnet: 172.20.0.0/16

# ? 容器网络拓扑
# ? 服务之间的连接关系
# ? 端口映射
```

---

### 9. **启动脚本和自动化配置**

**能保存：**
```dockerfile
COPY docker/entrypoint.sh /app/
RUN chmod +x /app/entrypoint.sh

ENTRYPOINT ["/app/entrypoint.sh"]

# ? 启动脚本
# ? 初始化逻辑
# ? 健康检查脚本
```

---

## ? Docker 无法保存的内容（需要额外配置）

### 1. **数据库数据**

**无法直接保存在镜像中！**

**原因：** 数据是动态的，不应该打包到镜像中。

**解决方案：** 使用 Docker 数据卷

```yaml
# docker-compose.yml
services:
  mysql:
    image: mysql:8.0
    volumes:
      - mysql_data:/var/lib/mysql  # ? 数据卷持久化

volumes:
  mysql_data:  # 数据卷定义
```

**新环境需要：**
```bash
# 1. 导入数据库结构
docker compose exec mysql mysql -u root -p < schema.sql

# 2. 或者导入完整备份
docker compose exec mysql mysql -u root -p < backup.sql
```

---

### 2. **Redis 数据**

**无法直接保存！**

**解决方案：** 使用数据卷 + 备份

```yaml
services:
  redis:
    image: redis:7-alpine
    volumes:
      - redis_data:/data  # ? 数据卷
```

**新环境需要：**
```bash
# Redis 通常是缓存，可以为空启动
# 如果需要恢复数据：
docker cp dump.rdb poor_redis:/data/
docker compose restart redis
```

---

### 3. **外部服务连接信息（敏感数据）**

**不应该保存在镜像中！**

**包括：**
- ? 数据库密码
- ? API 密钥
- ? 证书私钥
- ? 第三方服务凭证

**解决方案：** 使用环境变量或 Docker Secrets

```yaml
# docker-compose.yml
services:
  central:
    environment:
      MYSQL_PASSWORD: ${MYSQL_PASSWORD}  # 从 .env 读取
      API_KEY: ${API_KEY}
```

```bash
# .env 文件（不要提交到 Git！）
MYSQL_PASSWORD=your_password
API_KEY=your_api_key
```

**新环境需要：**
```bash
# 1. 创建 .env 文件
cat > .env <<EOF
MYSQL_ROOT_PASSWORD=your_password
MYSQL_DATABASE=gamedb
MYSQL_USER=gameuser
MYSQL_PASSWORD=game_password
EOF

# 2. 或使用 Docker Secrets（生产环境）
echo "my_password" | docker secret create mysql_password -
```

---

### 4. **日志文件**

**不应该保存在镜像中！**

**解决方案：** 使用数据卷或主机挂载

```yaml
services:
  central:
    volumes:
      - ./logs:/app/logs  # ? 日志存储在主机
```

**新环境需要：**
```bash
# 创建日志目录
mkdir -p logs
chmod 777 logs  # 或适当的权限
```

---

### 5. **用户上传的文件**

**不应该保存在镜像中！**

**解决方案：** 使用数据卷

```yaml
services:
  file:
    volumes:
      - file_storage:/app/data  # ? 文件存储
```

**新环境需要：**
```bash
# 如果需要迁移文件：
docker cp /path/to/files poor_file:/app/data/
```

---

### 6. **主机相关配置**

**无法保存：**
- ? 主机 IP 地址
- ? 主机端口（可能冲突）
- ? 主机文件路径
- ? 主机网络配置

**新环境需要：**
```bash
# 根据实际情况修改 docker-compose.yml
ports:
  - "13306:3306"  # 如果 3306 被占用
  
volumes:
  - /new/path/logs:/app/logs  # 使用新的主机路径
```

---

### 7. **外部依赖服务**

**无法包含在镜像中：**
- ? Windows 的 MySQL（外部服务）
- ? 云服务（AWS, Azure）
- ? 第三方 API 服务

**新环境需要：**
```bash
# 配置外部服务连接
environment:
  MYSQL_HOST: host.docker.internal  # Windows MySQL
  EXTERNAL_API: https://api.example.com
```

---

## ?? 新环境配置 Docker 后的完整步骤

### 前提条件

1. ? 已安装 Docker Desktop
2. ? 已克隆项目代码

### 完整步骤

#### **步骤 1：克隆项目**

```bash
git clone https://github.com/your-team/poor_server_stl.git
cd poor_server_stl
```

#### **步骤 2：配置环境变量（必需）**

```bash
# 创建 .env 文件
cat > .env <<EOF
# MySQL 配置
MYSQL_ROOT_PASSWORD=root123456
MYSQL_DATABASE=gamedb
MYSQL_USER=gameuser
MYSQL_PASSWORD=gamepass123

# Redis 配置（可选）
REDIS_PASSWORD=

# 服务器配置
SERVER_ENVIRONMENT=development
LOG_LEVEL=debug
EOF
```

**重要：** `.env` 文件不要提交到 Git！

#### **步骤 3：创建必要的目录（可选）**

```bash
# 创建日志目录
mkdir -p logs

# 创建数据备份目录
mkdir -p backups
```

#### **步骤 4：启动所有服务**

```bash
# 一键启动
docker compose up -d

# 查看启动状态
docker compose ps
```

**预期输出：**
```
NAME            IMAGE                  STATUS
poor_central    poor-server:latest     Up 30 seconds
poor_db         poor-server:latest     Up 30 seconds
poor_gateway    poor-server:latest     Up 30 seconds
poor_login      poor-server:latest     Up 30 seconds
poor_mysql      mysql:8.0              Up 1 minute (healthy)
poor_redis      redis:7-alpine         Up 1 minute (healthy)
```

#### **步骤 5：初始化数据库（必需）**

```bash
# 1. 等待 MySQL 完全启动
docker compose exec mysql mysqladmin ping -h localhost --wait=30

# 2. 创建数据库表结构
docker compose exec mysql mysql -u root -p${MYSQL_ROOT_PASSWORD} gamedb < docunment/server/数据库/sql定义文件/schema.sql

# 或者手动创建
docker compose exec mysql mysql -u root -p${MYSQL_ROOT_PASSWORD}
```

**SQL 示例：**
```sql
-- 创建必要的数据表
CREATE DATABASE IF NOT EXISTS gamedb CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE gamedb;

-- 创建用户表
CREATE TABLE IF NOT EXISTS users (
    user_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 其他表...
```

#### **步骤 6：验证服务**

```bash
# 1. 检查所有容器状态
docker compose ps

# 2. 检查日志
docker compose logs -f central

# 3. 测试 Redis 连接
docker compose exec redis redis-cli ping
# 输出: PONG

# 4. 测试 MySQL 连接
docker compose exec mysql mysql -u root -p${MYSQL_ROOT_PASSWORD} -e "SHOW DATABASES;"
```

#### **步骤 7：测试服务器连接**

```bash
# 使用客户端测试连接（根据你的项目）
# 例如：telnet 或 gRPC 客户端

# 查看服务器端口
docker compose ps
```

---

## ?? 实际案例：Poor Server STL 项目

### **Docker 保存了什么？**

#### ? **完整保存的内容**

```
1. 开发环境
   ├─ Ubuntu 22.04
   ├─ GCC 11.4
   ├─ CMake 3.28.3
   ├─ Ninja
   └─ Git

2. 所有第三方库（通过 vcpkg）
   ├─ gRPC
   ├─ Protobuf
   ├─ MySQL Connector C++
   ├─ cpp_redis + tacopie
   ├─ nlohmann-json
   ├─ lua
   ├─ spdlog
   ├─ jwt-cpp
   ├─ OpenSSL
   └─ Boost 库

3. 项目代码
   ├─ 所有源代码
   ├─ 编译好的可执行文件
   └─ 配置文件模板

4. 运行时环境
   ├─ MySQL 8.0 镜像
   ├─ Redis 7.0 镜像
   └─ 网络配置

5. 启动脚本
   ├─ entrypoint.sh
   └─ 服务管理脚本
```

#### ? **需要额外配置的内容**

```
1. 敏感信息（必须手动配置）
   ├─ 数据库密码 → 创建 .env 文件
   ├─ API 密钥 → 创建 .env 文件
   └─ 证书 → 手动导入

2. 数据库数据（需要初始化）
   ├─ 数据库结构 → 运行 SQL 脚本
   └─ 初始数据 → 导入数据文件

3. 持久化数据（使用数据卷）
   ├─ MySQL 数据 → 数据卷自动创建
   ├─ Redis 数据 → 数据卷自动创建
   └─ 日志文件 → 主机挂载

4. 主机特定配置（可能需要调整）
   ├─ 端口映射 → 根据主机情况修改
   └─ 文件路径 → 根据主机情况修改
```

---

### **新人配置完整清单**

#### **前置准备（5 分钟）**

- [ ] 安装 Docker Desktop
- [ ] 克隆项目代码
- [ ] 安装 Git（如果没有）

#### **环境配置（5 分钟）**

```bash
# 1. 进入项目目录
cd poor_server_stl

# 2. 创建 .env 文件
cat > .env <<EOF
MYSQL_ROOT_PASSWORD=root123456
MYSQL_DATABASE=gamedb
MYSQL_USER=gameuser
MYSQL_PASSWORD=gamepass123
EOF

# 3. 启动服务
docker compose up -d
```

#### **数据库初始化（5 分钟）**

```bash
# 1. 等待 MySQL 启动
sleep 30

# 2. 创建数据库表
docker compose exec -T mysql mysql -u root -proot123456 gamedb < schema.sql

# 或手动创建
docker compose exec mysql mysql -u root -proot123456
# 然后执行 CREATE TABLE ... 语句
```

#### **验证（2 分钟）**

```bash
# 1. 检查服务状态
docker compose ps

# 2. 查看日志
docker compose logs -f

# 3. 测试连接
# 根据你的客户端测试
```

**总耗时：约 15-20 分钟**（vs 传统方式的 2-3 天）

---

## ?? 新环境配置检查清单

### Docker 环境

- [ ] Docker Desktop 已安装并运行
- [ ] Docker Compose 可用
- [ ] Docker 版本 ≥ 20.10

### 项目文件

- [ ] 已克隆项目代码
- [ ] 可以访问 `Dockerfile` 和 `docker-compose.yml`
- [ ] 有网络连接（首次构建需要下载镜像）

### 配置文件

- [ ] 已创建 `.env` 文件
- [ ] 已配置数据库密码
- [ ] 已配置所有必需的环境变量

### 数据库

- [ ] 已准备 SQL 初始化脚本
- [ ] 了解数据库表结构
- [ ] 有测试数据（可选）

### 端口

- [ ] 确认端口不冲突（3306, 6379, 50051-50056, 8080-8082）
- [ ] 如有冲突，修改 `docker-compose.yml` 中的端口映射

### 存储

- [ ] 有足够磁盘空间（至少 10GB）
- [ ] 创建必要的目录（logs, backups）

---

## ?? 总结

### **Docker 保存的内容**

| 类别 | 内容 | 保存方式 |
|------|------|---------|
| **操作系统** | Ubuntu 22.04 | ? Dockerfile |
| **编译工具** | GCC, CMake, Ninja | ? Dockerfile |
| **第三方库** | vcpkg 所有依赖 | ? Dockerfile + vcpkg.json |
| **项目代码** | 源代码 + 可执行文件 | ? Dockerfile |
| **配置模板** | .lua 配置文件 | ? Dockerfile |
| **运行时环境** | MySQL, Redis 镜像 | ? docker-compose.yml |
| **网络配置** | 服务连接关系 | ? docker-compose.yml |

### **需要额外配置的内容**

| 类别 | 内容 | 配置方式 |
|------|------|---------|
| **敏感信息** | 密码、密钥 | ? .env 文件（手动创建） |
| **数据库数据** | 表结构、数据 | ? SQL 脚本（手动执行） |
| **持久化数据** | MySQL/Redis 数据 | ?? 数据卷（自动创建） |
| **日志文件** | 运行日志 | ?? 主机挂载（自动） |
| **用户文件** | 上传文件 | ?? 数据卷（自动创建） |

**图例：**
- ? = 完全自动化，无需配置
- ?? = 部分自动化，可能需要调整
- ? = 需要手动配置

---

### **新环境 3 步走**

```bash
# 步骤 1: 启动（5 分钟）
git clone <项目地址>
cd poor_server_stl
cat > .env <<EOF
MYSQL_ROOT_PASSWORD=root123456
MYSQL_DATABASE=gamedb
EOF
docker compose up -d

# 步骤 2: 初始化数据库（5 分钟）
docker compose exec -T mysql mysql -u root -proot123456 gamedb < schema.sql

# 步骤 3: 验证（2 分钟）
docker compose ps
docker compose logs -f

# 完成！
```

---

**Docker 将你的环境配置从 "黑魔法" 变成了 "三步搞定"！** ???

有了这份文档，新人配置环境将变得无比简单。所有复杂的环境依赖都被 Docker 优雅地解决了！
