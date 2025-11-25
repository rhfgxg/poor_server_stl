# Docker 环境共享完整指南

## ?? 核心价值：一次配置，到处运行

### Docker 的三大核心功能

#### 1?? **保存开发环境**

你现在 WSL2 中配置的所有环境，都可以通过 Dockerfile "快照"下来：

```dockerfile
FROM ubuntu:22.04

# 安装所有依赖（精确版本）
RUN apt-get update && apt-get install -y \
    gcc-11=11.4.0-1ubuntu1~22.04 \    # 精确版本
    g++-11=11.4.0-1ubuntu1~22.04 \
    cmake=3.22.1-1ubuntu1.22.04.2 \
    # ... 所有依赖
```

**优势：**
- ? **版本锁定**：每个依赖的版本都被记录
- ? **可重复**：任何人运行都得到相同环境
- ? **可审查**：Dockerfile 就是文档，一目了然

---

#### 2?? **快速分发给团队**

**场景：新同事入职**

**传统方式：**
```bash
# 新同事需要做的事情（可能需要 2-3 天）
1. 安装 WSL2
2. 安装 Ubuntu
3. 安装 GCC
4. 安装 CMake
5. 安装 vcpkg
6. 安装 MySQL
7. 安装 Redis
8. 配置 vcpkg
9. 编译项目依赖（可能失败）
10. 解决各种环境问题...
```

**Docker 方式：**
```bash
# 新同事只需要（5 分钟）
git clone https://github.com/your-team/poor_server_stl.git
cd poor_server_stl
docker compose up -d

# 完成！环境已就绪，可以开始开发
```

---

#### 3?? **持续集成/部署 (CI/CD)**

Docker 容器可以在任何地方运行：

```
开发环境           测试环境           生产环境
(你的电脑)         (CI 服务器)       (云服务器)
    ↓                  ↓                 ↓
相同的 Docker 镜像 → 相同的 Docker 镜像 → 相同的 Docker 镜像

结果：环境完全一致！
```

---

## ?? **完整工作流程示例**

### 场景：项目开发和部署

#### **阶段 1：你（开发者 A）创建环境**

```bash
# 1. 在项目根目录创建 Dockerfile
cat > Dockerfile <<'EOF'
FROM ubuntu:22.04

# 设置时区和语言
ENV TZ=Asia/Shanghai LANG=C.UTF-8

# 安装开发工具
RUN apt-get update && apt-get install -y \
    gcc-11 g++-11 cmake ninja-build \
    git wget curl pkg-config \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100

# 安装 vcpkg
WORKDIR /opt
RUN git clone https://github.com/microsoft/vcpkg.git && \
    cd vcpkg && ./bootstrap-vcpkg.sh

# 复制项目文件
COPY . /app
WORKDIR /app

# 安装项目依赖
RUN /opt/vcpkg/vcpkg install --triplet=x64-linux

# 编译项目
RUN mkdir build && cd build && \
    cmake -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    .. && ninja
EOF

# 2. 构建镜像
docker build -t poor-server-dev:v1.0 .

# 3. 推送到 Docker Hub（可选）
docker tag poor-server-dev:v1.0 yourname/poor-server-dev:v1.0
docker push yourname/poor-server-dev:v1.0
```

#### **阶段 2：新同事（开发者 B）使用环境**

```bash
# 方式 1：从 Docker Hub 拉取
docker pull yourname/poor-server-dev:v1.0
docker run -it yourname/poor-server-dev:v1.0 bash

# 方式 2：从项目源码构建（如果你提交了 Dockerfile）
git clone <项目地址>
cd poor_server_stl
docker compose up -d  # 自动构建并启动
```

---

## ?? **实际案例：你的项目环境保存**

### **当前 WSL2 环境 → Docker 镜像**

我已经为你的项目创建了完整的 Dockerfile，它包含了你 WSL2 中的所有环境：

#### **Dockerfile 内容解析**

```dockerfile
# ============ 阶段 1: 构建阶段 ============
FROM ubuntu:22.04 AS builder

# 1. 安装构建依赖（与你的 WSL2 环境一致）
RUN apt-get update && apt-get install -y \
    build-essential \      # ← 包含 GCC, make 等
    cmake \                # ← CMake 3.22+
    ninja-build \          # ← Ninja 构建工具
    gcc-11 g++-11          # ← GCC 11（与你的版本一致）

# 2. 安装 vcpkg（与你手动安装的一样）
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg && \
    /opt/vcpkg/bootstrap-vcpkg.sh

# 3. 复制项目并安装依赖
COPY . /build
RUN /opt/vcpkg/vcpkg install --triplet=x64-linux

# 4. 编译项目
RUN cmake ... && ninja

# ============ 阶段 2: 运行阶段 ============
FROM ubuntu:22.04 AS runtime

# 只安装运行时依赖（镜像更小）
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    libprotobuf23 \
    # ... 只要运行时库

# 复制编译好的程序
COPY --from=builder /build/build/src/*/bin /app/bin/
```

**这个 Dockerfile 完整记录了：**
- ? 操作系统版本（Ubuntu 22.04）
- ? 编译器版本（GCC 11）
- ? 所有依赖库及版本
- ? vcpkg 配置
- ? 编译步骤
- ? 运行时环境

---

## ?? **新同事如何使用**

### **完整步骤（5 分钟）**

```bash
# 1. 克隆项目
git clone https://github.com/your-team/poor_server_stl.git
cd poor_server_stl

# 2. 一键启动所有服务
docker compose up -d

# 就这么简单！

# 查看运行状态
docker compose ps

# 查看日志
docker compose logs -f

# 进入开发环境
docker compose exec central bash
```

### **docker-compose.yml 的作用**

```yaml
# docker-compose.yml 定义了完整的开发/生产环境

services:
  # MySQL 数据库（自动配置）
  mysql:
    image: mysql:8.0
    environment:
      MYSQL_ROOT_PASSWORD: auto-configured
    # 无需手动安装！

  # Redis 缓存（自动配置）
  redis:
    image: redis:7-alpine
    # 无需手动安装！

  # 游戏服务器（使用你的 Dockerfile）
  central:
    build: .  # ← 自动构建你的环境
    depends_on:
      - mysql
      - redis
    # 自动连接所有服务！
```

**新同事得到的环境：**
- ? MySQL 8.0（自动安装和配置）
- ? Redis 7.0（自动安装和配置）
- ? 所有游戏服务器（自动编译和启动）
- ? 网络配置（服务之间自动连接）
- ? 数据持久化（自动配置数据卷）

---

## ?? **环境对比**

| 项目 | 传统方式 | Docker 方式 |
|------|---------|------------|
| **配置时间** | 2-3 天 | 5 分钟 |
| **环境一致性** | ? 无法保证 | ? 100% 一致 |
| **依赖冲突** | ? 经常发生 | ? 完全隔离 |
| **新人上手** | ? 需要文档+指导 | ? 一键启动 |
| **版本管理** | ? 手动记录 | ? 自动记录（Dockerfile） |
| **回滚** | ? 困难 | ? 简单（切换镜像版本） |
| **部署** | ? 每次重新配置 | ? 直接使用镜像 |

---

## ?? **具体使用场景**

### **场景 1：新人入职**

**你（项目负责人）：**
```bash
# 将 Dockerfile 和 docker-compose.yml 提交到 Git
git add Dockerfile docker-compose.yml
git commit -m "Add Docker support"
git push
```

**新同事：**
```bash
git clone <项目地址>
cd poor_server_stl

# 启动开发环境
docker compose up -d

# 进入开发容器
docker compose exec central bash

# 开始开发！
```

### **场景 2：版本发布**

```bash
# 打标签
docker tag poor-server:latest poor-server:v1.0.0

# 推送到镜像仓库
docker push poor-server:v1.0.0

# 在生产服务器上
docker pull poor-server:v1.0.0
docker run -d poor-server:v1.0.0
```

### **场景 3：回退版本**

```bash
# 发现 v1.1.0 有 bug，回退到 v1.0.0
docker stop current_server
docker run -d poor-server:v1.0.0
# 完成！环境完全恢复到 v1.0.0
```

### **场景 4：多环境测试**

```bash
# 同时运行多个版本测试
docker run -d -p 8001:8000 poor-server:v1.0.0
docker run -d -p 8002:8000 poor-server:v1.1.0
docker run -d -p 8003:8000 poor-server:dev

# 对比测试不同版本
```

---

## ?? **镜像分发方式**

### **方式 1：Docker Hub（公开/私有）**

```bash
# 注册 Docker Hub 账号（免费）
# https://hub.docker.com/

# 登录
docker login

# 推送镜像
docker push yourname/poor-server:latest

# 团队成员拉取
docker pull yourname/poor-server:latest
```

### **方式 2：私有镜像仓库**

```bash
# 公司内网搭建 Harbor 或其他私有仓库
docker tag poor-server:latest registry.company.com/poor-server:latest
docker push registry.company.com/poor-server:latest
```

### **方式 3：导出/导入文件**

```bash
# 保存镜像为文件（适合无网络环境）
docker save poor-server:latest -o poor-server.tar

# 传输文件给同事（U盘、网盘等）

# 同事加载镜像
docker load -i poor-server.tar
```

---

## ?? **最佳实践**

### **1. Dockerfile 版本控制**

```bash
# 将 Dockerfile 提交到 Git
poor_server_stl/
├─ Dockerfile              ← 提交到 Git
├─ docker-compose.yml      ← 提交到 Git
├─ .dockerignore           ← 提交到 Git
└─ src/
```

### **2. 使用 .dockerignore**

```bash
# .dockerignore（类似 .gitignore）
build/
*.log
.git/
.vs/
vcpkg_installed/
```

### **3. 镜像标签策略**

```bash
# 使用语义化版本
docker tag poor-server:latest poor-server:v1.0.0
docker tag poor-server:latest poor-server:v1.0
docker tag poor-server:latest poor-server:stable

# 开发版本
docker tag poor-server:latest poor-server:dev
docker tag poor-server:latest poor-server:feature-xxx
```

---

## ?? **总结**

### **Docker 可以为你的项目做什么？**

? **1. 环境快照**
- 将你的 WSL2 环境完整保存在 Dockerfile 中
- 任何人都能精确复现你的环境

? **2. 快速分发**
- 新同事 5 分钟启动完整开发环境
- 告别"在我电脑上能运行"的问题

? **3. 版本管理**
- 每个版本的环境都可以保存
- 随时回退到任何历史版本

? **4. CI/CD**
- 自动化测试和部署
- 开发、测试、生产环境完全一致

? **5. 隔离性**
- 不同项目互不干扰
- 可以在同一台机器上运行多个版本

---

### **下一步行动**

```bash
# 1. 你的项目已经有完整的 Docker 配置
cd /mnt/d/Project/cpp/poor/poor_server_stl

# 2. 测试 Docker 环境
docker compose up -d

# 3. 验证所有服务正常运行
docker compose ps
docker compose logs -f

# 4. 将 Docker 配置提交到 Git
git add Dockerfile docker-compose.yml docker/
git commit -m "Add Docker support for easy environment setup"
git push

# 5. 告诉你的团队
# 新同事只需要：
#   git clone <项目地址>
#   docker compose up -d
#   开始开发！
```

---

**你的理解完全正确！Docker 就是为了解决"环境一致性"问题而生的。** ???

有了 Docker，你的项目环境配置将从"黑魔法"变成"点击即用"！
