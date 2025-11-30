# Docker 环境配置指南

本文档介绍如何在 Windows + WSL2 环境下配置 Docker，用于容器化部署 poor_server_stl 项目。

---

## ?? 目录

1. [安装 Docker Desktop](#安装-docker-desktop)
2. [验证安装](#验证安装)
3. [Docker 基础命令](#docker-基础命令)
4. [项目容器化](#项目容器化)

---

## ?? 安装 Docker Desktop

### 步骤 1：下载 Docker Desktop

访问官方网站下载：
- 官网：https://www.docker.com/products/docker-desktop/
- 直接下载：https://desktop.docker.com/win/main/amd64/Docker%20Desktop%20Installer.exe

### 步骤 2：系统要求

- Windows 10/11 专业版、企业版或教育版（需要 Hyper-V）
- 或者 Windows 10/11 家庭版 + WSL2（推荐）
- 至少 4GB RAM
- 启用 BIOS 虚拟化

### 步骤 3：安装前准备

**确保 WSL2 已安装并更新：**

```powershell
# 以管理员身份运行 PowerShell

# 检查 WSL 版本
wsl --version

# 如果需要，更新 WSL
wsl --update

# 设置 WSL2 为默认版本
wsl --set-default-version 2
```

### 步骤 4：安装 Docker Desktop

1. 运行下载的安装程序
2. 选择 "Use WSL 2 instead of Hyper-V"（推荐）
3. 按照向导完成安装
4. 重启计算机

### 步骤 5：配置 Docker Desktop

安装后首次启动：

1. **接受服务条款**
2. **登录 Docker Hub**（可选，建议注册一个免费账号）
3. **设置 → Resources → WSL Integration**
   - 启用 "Enable integration with my default WSL distro"
   - 勾选你的 Ubuntu WSL2 发行版

4. **设置 → Resources → Advanced**（可选优化）
   ```
   CPUs: 4-6 核（根据你的 CPU）
   Memory: 4-8 GB（根据你的内存）
   Swap: 1-2 GB
   Disk image size: 60 GB+
   ```

5. **应用并重启**

---

## ? 验证安装

### 在 PowerShell 中验证

```powershell
# 检查 Docker 版本
docker --version
# 输出示例: Docker version 24.0.7, build afdd53b

# 检查 Docker Compose 版本
docker compose version
# 输出示例: Docker Compose version v2.23.0

# 运行测试容器
docker run hello-world
```

### 在 WSL2 中验证

```bash
# 启动 WSL2
wsl

# 检查 Docker
docker --version

# 运行测试容器
docker run hello-world

# 查看运行的容器
docker ps

# 查看所有容器（包括停止的）
docker ps -a
```

**预期输出：**
```
CONTAINER ID   IMAGE         COMMAND    CREATED         STATUS                     PORTS     NAMES
abc123def456   hello-world   "/hello"   2 minutes ago   Exited (0) 2 minutes ago             amazing_name
```

---

## ?? Docker 基础命令

### 镜像管理

```bash
# 搜索镜像
docker search ubuntu

# 拉取镜像
docker pull ubuntu:22.04

# 列出本地镜像
docker images

# 删除镜像
docker rmi ubuntu:22.04

# 构建镜像
docker build -t myapp:latest .

# 给镜像打标签
docker tag myapp:latest myapp:v1.0
```

### 容器管理

```bash
# 运行容器
docker run -it ubuntu:22.04 bash
docker run -d -p 8080:80 nginx  # 后台运行，端口映射

# 列出运行中的容器
docker ps

# 列出所有容器
docker ps -a

# 停止容器
docker stop <container_id>

# 启动已停止的容器
docker start <container_id>

# 重启容器
docker restart <container_id>

# 删除容器
docker rm <container_id>

# 进入运行中的容器
docker exec -it <container_id> bash

# 查看容器日志
docker logs <container_id>
docker logs -f <container_id>  # 持续输出日志

# 查看容器资源使用
docker stats
```

### 数据管理

```bash
# 创建数据卷
docker volume create mydata

# 列出数据卷
docker volume ls

# 删除数据卷
docker volume rm mydata

# 使用数据卷运行容器
docker run -v mydata:/data ubuntu:22.04
```

### 网络管理

```bash
# 列出网络
docker network ls

# 创建网络
docker network create mynetwork

# 在指定网络中运行容器
docker run --network mynetwork ubuntu:22.04

# 删除网络
docker network rm mynetwork
```

### 清理命令

```bash
# 删除所有停止的容器
docker container prune

# 删除所有未使用的镜像
docker image prune

# 删除所有未使用的数据卷
docker volume prune

# 清理所有未使用的资源
docker system prune -a
```

---

## ?? 项目容器化

接下来将在单独的文档中详细介绍如何容器化 poor_server_stl 项目。

参见：
- [Docker_部署指南.md](./Docker_部署指南.md) - 如何将项目容器化
- [Dockerfile 详解](./Dockerfile) - 项目的 Dockerfile 配置

---

## ?? 常见问题

### Q: Docker Desktop 启动失败

**A: 检查以下几点**

1. **WSL2 是否正确安装**
   ```powershell
   wsl --status
   ```

2. **虚拟化是否启用**
   - 重启电脑，进入 BIOS
   - 启用 Intel VT-x 或 AMD-V

3. **查看 Docker 日志**
   ```
   C:\Users\你的用户名\AppData\Local\Docker\log.txt
   ```

### Q: "WSL 2 installation is incomplete" 错误

**A: 安装 WSL2 Linux 内核更新包**

下载并安装：https://aka.ms/wsl2kernel

### Q: Docker 容器无法访问网络

**A: 重置 Docker 网络**

```bash
docker network prune
docker restart
```

### Q: 容器内无法访问 Windows 文件

**A: 使用 WSL2 路径挂载**

```bash
# 错误方式
docker run -v D:/Project:/app ...

# 正确方式
docker run -v /mnt/d/Project:/app ...
```

---

## ?? 学习资源

### 官方文档
- Docker 官方文档：https://docs.docker.com/
- Docker Hub：https://hub.docker.com/

### 推荐教程
- Docker 入门到实践：https://yeasy.gitbook.io/docker_practice/
- Docker 官方入门教程：https://docs.docker.com/get-started/

### 视频教程
- Docker 从入门到精通（B站）
- Docker 实战教程（YouTube）

---

## ?? 下一步

安装完成后，参考以下文档继续学习：

1. **创建第一个 Dockerfile** - [Dockerfile_入门.md](./Dockerfile_入门.md)
2. **使用 Docker Compose** - [Docker_Compose_指南.md](./Docker_Compose_指南.md)
3. **部署游戏服务器** - [Docker_部署指南.md](./Docker_部署指南.md)

---

**最后更新：** 2024-11-25
