# Docker 部署指南 - Poor Server STL

本文档详细介绍如何使用 Docker 容器化部署 Poor Server STL 游戏服务器。

---

## ?? 目录

1. [快速开始](#快速开始)
2. [使用 Docker Compose](#使用-docker-compose)
3. [单独使用 Docker](#单独使用-docker)
4. [配置说明](#配置说明)
5. [常用命令](#常用命令)
6. [故障排查](#故障排查)

---

## ?? 快速开始

### 前提条件

- ? 已安装 Docker Desktop（参见 [Docker_环境配置指南.md](./Docker_环境配置指南.md)）
- ? Docker Compose 已安装（Docker Desktop 自带）
- ? 项目代码已克隆到本地

### 一键启动（推荐）

```bash
# 1. 进入项目目录
cd /mnt/d/Project/cpp/poor/poor_server_stl

# 2. 构建并启动所有服务
docker compose up -d

# 3. 查看服务状态
docker compose ps

# 4. 查看日志
docker compose logs -f
```

**就是这么简单！** 所有服务（包括 MySQL、Redis、游戏服务器）都会自动启动。

---

## ?? 使用 Docker Compose（推荐）

Docker Compose 可以一次性管理多个容器。

### 基本命令

```bash
# 启动所有服务（后台运行）
docker compose up -d

# 启动所有服务（前台运行，可以看到日志）
docker compose up

# 停止所有服务
docker compose stop

# 停止并删除所有容器
docker compose down

# 停止并删除容器、网络、数据卷
docker compose down -v

# 重启所有服务
docker compose restart

# 查看服务状态
docker compose ps

# 查看日志
docker compose logs

# 实时跟踪日志
docker compose logs -f

# 查看特定服务的日志
docker compose logs -f central
docker compose logs -f gateway
```

### 启动特定服务

```bash
# 只启动 MySQL 和 Redis
docker compose up -d mysql redis

# 只启动游戏服务器（不包括数据库）
docker compose up -d central db login gateway file matching

# 启动单个服务
docker compose up -d central
```

### 重新构建镜像

```bash
# 重新构建所有镜像
docker compose build

# 重新构建并启动
docker compose up -d --build

# 重新构建特定服务
docker compose build central
```

### 扩展服务（负载均衡）

```bash
# 启动 3 个 gateway 实例
docker compose up -d --scale gateway=3

# 启动 2 个 matching 实例
docker compose up -d --scale matching=2
```

---

## ?? 单独使用 Docker

如果不使用 Docker Compose，可以单独运行容器。

### 1. 构建镜像

```bash
# 构建游戏服务器镜像
docker build -t poor-server:latest .

# 构建并指定标签
docker build -t poor-server:v1.0 -t poor-server:latest .
```

### 2. 启动 MySQL

```bash
docker run -d \
  --name poor_mysql \
  -e MYSQL_ROOT_PASSWORD=root123456 \
  -e MYSQL_DATABASE=gamedb \
  -e MYSQL_USER=gameuser \
  -e MYSQL_PASSWORD=gamepass123 \
  -p 3306:3306 \
  -p 33060:33060 \
  -v mysql_data:/var/lib/mysql \
  mysql:8.0
```

### 3. 启动 Redis

```bash
docker run -d \
  --name poor_redis \
  -p 6379:6379 \
  -v redis_data:/data \
  redis:7-alpine
```

### 4. 启动游戏服务器

```bash
# 创建网络
docker network create game_network

# 连接 MySQL 和 Redis 到网络
docker network connect game_network poor_mysql
docker network connect game_network poor_redis

# 启动 Central 服务
docker run -d \
  --name poor_central \
  --network game_network \
  -e REDIS_HOST=poor_redis \
  -e MYSQL_HOST=poor_mysql \
  -p 50051:50051 \
  -v $(pwd)/logs:/app/logs \
  poor-server:latest central

# 启动其他服务（类似）
docker run -d --name poor_db --network game_network -p 50052:50052 poor-server:latest db
docker run -d --name poor_login --network game_network -p 50053:50053 poor-server:latest login
docker run -d --name poor_gateway --network game_network -p 50054:50054 poor-server:latest gateway
docker run -d --name poor_file --network game_network -p 50055:50055 poor-server:latest file
docker run -d --name poor_matching --network game_network -p 50056:50056 poor-server:latest matching
```

---

## ?? 配置说明

### 环境变量

Docker Compose 使用环境变量来配置服务。创建 `.env` 文件：

```bash
# .env 文件

# MySQL 配置
MYSQL_ROOT_PASSWORD=your_root_password
MYSQL_DATABASE=gamedb
MYSQL_USER=gameuser
MYSQL_PASSWORD=your_game_password

# Redis 配置
REDIS_PASSWORD=your_redis_password

# 服务器配置
SERVER_ENVIRONMENT=production
LOG_LEVEL=info
```

### 端口映射

默认端口映射：

| 服务 | 容器端口 | 主机端口 | 说明 |
|------|---------|---------|------|
| MySQL | 3306 | 3306 | MySQL 标准端口 |
| MySQL X | 33060 | 33060 | MySQL X Protocol |
| Redis | 6379 | 6379 | Redis 端口 |
| Central | 50051 | 50051 | 中心服务器 |
| DB | 50052 | 50052 | 数据库服务器 |
| Login | 50053 | 50053 | 登录服务器 |
| Gateway | 50054 | 50054 | 网关服务器 |
| File | 50055 | 50055 | 文件服务器 |
| Matching | 50056 | 50056 | 匹配服务器 |
| Redis UI | 8081 | 8081 | Redis 管理面板 |
| phpMyAdmin | 80 | 8082 | MySQL 管理面板 |

修改端口：在 `docker-compose.yml` 中修改 `ports` 配置。

### 数据持久化

Docker Compose 使用数据卷来持久化数据：

```yaml
volumes:
  mysql_data:      # MySQL 数据
  redis_data:      # Redis 数据
  file_storage:    # 文件存储
```

**查看数据卷：**

```bash
docker volume ls

# 查看数据卷详细信息
docker volume inspect poor_server_stl_mysql_data

# 备份数据卷
docker run --rm -v poor_server_stl_mysql_data:/data -v $(pwd):/backup \
  ubuntu tar czf /backup/mysql_backup.tar.gz /data
```

---

## ?? 常用命令

### 查看容器状态

```bash
# 查看所有运行的容器
docker ps

# 查看所有容器（包括停止的）
docker ps -a

# 查看容器详细信息
docker inspect poor_central

# 查看容器资源使用情况
docker stats

# 查看容器实时日志
docker logs -f poor_central
```

### 进入容器

```bash
# 进入容器 Shell
docker exec -it poor_central bash

# 在容器中执行命令
docker exec poor_central ps aux
docker exec poor_central ls -la /app/bin
```

### 调试容器

```bash
# 查看容器日志
docker logs poor_central

# 查看最后 100 行日志
docker logs --tail 100 poor_central

# 查看容器进程
docker top poor_central

# 查看容器文件系统变化
docker diff poor_central

# 导出容器为镜像
docker commit poor_central poor-server:snapshot
```

### 网络管理

```bash
# 查看网络
docker network ls

# 查看网络详细信息
docker network inspect game_network

# 连接容器到网络
docker network connect game_network my_container

# 断开容器网络
docker network disconnect game_network my_container
```

### 清理资源

```bash
# 停止所有容器
docker stop $(docker ps -aq)

# 删除所有停止的容器
docker container prune

# 删除未使用的镜像
docker image prune

# 删除未使用的数据卷
docker volume prune

# 清理所有未使用的资源（危险！）
docker system prune -a --volumes
```

---

## ?? 故障排查

### 问题 1：容器无法启动

**症状：** 容器一启动就退出

**排查步骤：**

```bash
# 1. 查看容器日志
docker logs poor_central

# 2. 查看容器退出代码
docker ps -a

# 3. 尝试以交互模式运行
docker run -it --rm poor-server:latest bash

# 4. 检查依赖服务是否运行
docker compose ps
```

### 问题 2：无法连接到 MySQL/Redis

**症状：** 服务启动后报数据库连接错误

**解决方法：**

```bash
# 1. 检查 MySQL 是否健康
docker inspect poor_mysql | grep -i health

# 2. 手动测试连接
docker exec poor_central ping mysql
docker exec poor_central redis-cli -h redis ping

# 3. 查看网络配置
docker network inspect game_network

# 4. 等待服务就绪后再启动
docker compose up -d mysql redis
# 等待 30 秒
sleep 30
docker compose up -d central db login gateway file matching
```

### 问题 3：端口冲突

**症状：** `Bind for 0.0.0.0:3306 failed: port is already allocated`

**解决方法：**

```bash
# 1. 查看端口占用
netstat -ano | findstr :3306  # Windows
lsof -i :3306  # Linux/Mac

# 2. 修改 docker-compose.yml 中的端口映射
# ports:
#   - "13306:3306"  # 使用主机的 13306 端口

# 3. 或者停止占用端口的服务
```

### 问题 4：磁盘空间不足

**症状：** `no space left on device`

**解决方法：**

```bash
# 查看 Docker 磁盘使用
docker system df

# 清理未使用的资源
docker system prune -a

# 删除特定镜像
docker rmi poor-server:old-version

# 删除悬空镜像
docker image prune
```

### 问题 5：构建镜像失败

**症状：** `docker build` 失败

**解决方法：**

```bash
# 1. 查看详细构建日志
docker build --no-cache --progress=plain -t poor-server:latest .

# 2. 检查 Dockerfile 语法
# 3. 确保网络连接正常（下载依赖）
# 4. 增加 Docker 资源限制（内存、CPU）
```

---

## ?? 生产环境部署建议

### 1. 使用 Docker Secrets 管理敏感信息

```bash
# 创建密钥
echo "my_secret_password" | docker secret create mysql_root_password -

# 在 docker-compose.yml 中使用
secrets:
  mysql_root_password:
    external: true
```

### 2. 启用日志轮转

```yaml
services:
  central:
    logging:
      driver: "json-file"
      options:
        max-size: "10m"
        max-file: "3"
```

### 3. 配置健康检查

```yaml
healthcheck:
  test: ["CMD", "pgrep", "-x", "central"]
  interval: 30s
  timeout: 10s
  retries: 3
  start_period: 40s
```

### 4. 使用反向代理（Nginx）

```yaml
services:
  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
```

---

## ?? 下一步学习

1. **Docker 网络深入** - 了解 bridge、host、overlay 网络
2. **Docker 卷管理** - 数据持久化和备份
3. **Docker Swarm** - 容器编排和集群管理
4. **Kubernetes** - 生产级容器编排平台

---

## ?? 相关资源

- [Dockerfile](../Dockerfile) - 镜像构建配置
- [docker-compose.yml](../docker-compose.yml) - 服务编排配置
- [entrypoint.sh](../docker/entrypoint.sh) - 容器启动脚本
- [Docker 环境配置指南](./Docker_环境配置指南.md)

---

**最后更新：** 2024-11-25
