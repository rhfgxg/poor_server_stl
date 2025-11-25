# Docker 快速参考 - Poor Server STL

一页纸速查表，包含最常用的 Docker 命令。

---

## ?? 快速开始

```bash
# 一键启动所有服务
docker compose up -d

# 查看状态
docker compose ps

# 查看日志
docker compose logs -f

# 停止所有服务
docker compose down
```

---

## ?? Docker Compose 常用命令

| 命令 | 说明 |
|------|------|
| `docker compose up -d` | 后台启动所有服务 |
| `docker compose down` | 停止并删除所有容器 |
| `docker compose ps` | 查看服务状态 |
| `docker compose logs -f [service]` | 查看日志 |
| `docker compose restart [service]` | 重启服务 |
| `docker compose build` | 重新构建镜像 |
| `docker compose up -d --build` | 重建并启动 |
| `docker compose exec [service] bash` | 进入容器 Shell |
| `docker compose stop` | 停止服务 |
| `docker compose start` | 启动服务 |

---

## ?? Docker 基础命令

### 容器管理

```bash
# 查看运行的容器
docker ps

# 查看所有容器
docker ps -a

# 启动/停止/重启容器
docker start <container>
docker stop <container>
docker restart <container>

# 删除容器
docker rm <container>
docker rm -f <container>  # 强制删除

# 进入容器
docker exec -it <container> bash
docker exec <container> <command>

# 查看日志
docker logs <container>
docker logs -f <container>  # 实时跟踪
docker logs --tail 100 <container>  # 最后100行
```

### 镜像管理

```bash
# 列出镜像
docker images

# 构建镜像
docker build -t <name>:<tag> .

# 删除镜像
docker rmi <image>

# 拉取/推送镜像
docker pull <image>
docker push <image>

# 给镜像打标签
docker tag <source> <target>
```

### 资源清理

```bash
# 删除停止的容器
docker container prune

# 删除未使用的镜像
docker image prune

# 删除未使用的卷
docker volume prune

# 清理所有未使用资源
docker system prune -a
```

---

## ?? 调试命令

```bash
# 查看容器详细信息
docker inspect <container>

# 查看容器进程
docker top <container>

# 查看容器资源使用
docker stats

# 查看容器文件系统变化
docker diff <container>

# 从容器复制文件
docker cp <container>:/path/to/file ./local/path
docker cp ./local/file <container>:/path/to/file
```

---

## ?? 网络命令

```bash
# 列出网络
docker network ls

# 查看网络详情
docker network inspect <network>

# 创建网络
docker network create <network>

# 连接/断开容器
docker network connect <network> <container>
docker network disconnect <network> <container>
```

---

## ?? 数据卷命令

```bash
# 列出数据卷
docker volume ls

# 查看数据卷详情
docker volume inspect <volume>

# 创建数据卷
docker volume create <volume>

# 删除数据卷
docker volume rm <volume>

# 备份数据卷
docker run --rm -v <volume>:/data -v $(pwd):/backup \
  ubuntu tar czf /backup/backup.tar.gz /data
```

---

## ?? 项目特定命令

### 服务管理

```bash
# 启动特定服务
docker compose up -d central
docker compose up -d mysql redis

# 查看特定服务日志
docker compose logs -f central
docker compose logs -f gateway

# 重启特定服务
docker compose restart central

# 扩展服务实例
docker compose up -d --scale gateway=3
```

### 数据库操作

```bash
# 连接 MySQL
docker compose exec mysql mysql -u root -p

# 连接 Redis
docker compose exec redis redis-cli

# 备份 MySQL
docker compose exec mysql mysqldump -u root -p gamedb > backup.sql

# 恢复 MySQL
docker compose exec -T mysql mysql -u root -p gamedb < backup.sql
```

### 监控和管理

```bash
# Redis 管理界面
# 浏览器访问: http://localhost:8081

# phpMyAdmin
# 浏览器访问: http://localhost:8082
```

---

## ?? 常见问题快速修复

| 问题 | 解决命令 |
|------|---------|
| 端口被占用 | `docker compose down` 然后修改端口 |
| 容器一直重启 | `docker logs <container>` 查看错误 |
| 磁盘空间不足 | `docker system prune -a` |
| 网络连接问题 | `docker network inspect game_network` |
| 数据库连接失败 | 等待健康检查: `docker compose ps` |
| 重新构建镜像 | `docker compose up -d --build` |

---

## ?? 健康检查

```bash
# 查看服务健康状态
docker compose ps

# 查看容器健康检查日志
docker inspect --format='{{json .State.Health}}' <container> | jq

# 手动执行健康检查
docker compose exec central pgrep -x central
docker compose exec redis redis-cli ping
docker compose exec mysql mysqladmin ping
```

---

## ?? 日志管理

```bash
# 查看所有服务日志
docker compose logs

# 实时跟踪所有日志
docker compose logs -f

# 查看特定服务最近日志
docker compose logs --tail=50 central

# 按时间过滤日志
docker compose logs --since 30m central

# 导出日志
docker compose logs > logs.txt
```

---

## ?? 紧急操作

```bash
# 立即停止所有服务
docker compose kill

# 强制删除所有容器
docker compose rm -f

# 完全重置（危险！）
docker compose down -v  # 删除数据卷
docker system prune -a  # 清理所有资源
```

---

## ?? 环境变量

```bash
# 查看容器环境变量
docker compose exec central env

# 运行时设置环境变量
docker compose run -e DEBUG=true central

# 使用 .env 文件
# 创建 .env 文件后，docker compose 会自动加载
```

---

## ?? 有用的链接

- Docker 官方文档: https://docs.docker.com/
- Docker Hub: https://hub.docker.com/
- Docker Compose 文档: https://docs.docker.com/compose/

---

**提示：** 
- 使用 `--help` 查看命令详细帮助，如 `docker compose --help`
- 大多数命令支持简写，如 `docker ps` 等同于 `docker container ls`
- 善用 Tab 键自动补全容器名和镜像名

---

**快捷键：**
- `Ctrl+C` - 停止实时日志输出
- `Ctrl+D` - 退出容器 Shell
- `Ctrl+P, Ctrl+Q` - 分离容器但保持运行

---

**最后更新：** 2024-11-25
