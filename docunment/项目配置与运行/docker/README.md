# Docker 文档索引

本目录包含 Poor Server STL 项目的 Docker 相关文档。

---

## ?? 文档列表

### ?? 入门教程

1. **[Docker_环境配置指南.md](./Docker_环境配置指南.md)**
   - Docker Desktop 安装步骤
   - WSL2 配置
   - 基础命令教程
   - 常见问题解决
   - **适合对象：** 从未使用过 Docker 的新手

2. **[Docker_能保存什么.md](./Docker_能保存什么.md)** ? **重要**
   - Docker 能保存的内容详解
   - Docker 无法保存的内容
   - 新环境配置完整步骤
   - 配置检查清单
   - **适合对象：** 想了解 Docker 原理的开发者

---

### ?? 实战指南

3. **[Docker_部署指南.md](./Docker_部署指南.md)**
   - 快速开始
   - Docker Compose 使用
   - 单独使用 Docker
   - 生产环境部署建议
   - **适合对象：** 需要部署项目的开发者

4. **[Docker_环境共享指南.md](./Docker_环境共享指南.md)**
   - 环境快照和共享
   - 团队协作方式
   - 镜像分发方法
   - 完整工作流程
   - **适合对象：** 团队负责人和项目维护者

---

### ?? 参考手册

5. **[Docker_快速参考.md](./Docker_快速参考.md)**
   - 常用命令速查表
   - 项目特定命令
   - 故障快速修复
   - 快捷键和技巧
   - **适合对象：** 日常开发查阅

---

## ??? 学习路线

### 新手入门（第 1-2 天）

```
1. 阅读 Docker_环境配置指南.md
   └─ 安装 Docker Desktop
   └─ 运行第一个容器

2. 阅读 Docker_能保存什么.md
   └─ 理解 Docker 原理
   └─ 了解能保存和不能保存的内容

3. 实践：启动项目
   └─ docker compose up -d
   └─ 查看运行状态
```

### 进阶使用（第 3-5 天）

```
1. 阅读 Docker_部署指南.md
   └─ 掌握 Docker Compose
   └─ 学习数据持久化
   └─ 了解网络配置

2. 阅读 Docker_环境共享指南.md
   └─ 学习镜像分发
   └─ 理解团队协作流程
```

### 日常开发

```
使用 Docker_快速参考.md
   └─ 快速查找命令
   └─ 解决常见问题
```

---

## ?? 快速导航

### 根据需求查找文档

| 需求 | 推荐文档 |
|------|---------|
| 我是新手，想安装 Docker | [Docker_环境配置指南.md](./Docker_环境配置指南.md) |
| 我想知道 Docker 能做什么 | [Docker_能保存什么.md](./Docker_能保存什么.md) ? |
| 我想快速启动项目 | [Docker_部署指南.md](./Docker_部署指南.md) |
| 我想分享环境给团队 | [Docker_环境共享指南.md](./Docker_环境共享指南.md) |
| 我忘记命令了 | [Docker_快速参考.md](./Docker_快速参考.md) |

---

## ?? 相关文件

### 配置文件

在项目根目录：

```
poor_server_stl/
├── Dockerfile                    # 镜像构建配置
├── docker-compose.yml            # 服务编排配置
├── .dockerignore                 # Docker 忽略文件
└── docker/
    ├── entrypoint.sh             # 容器启动脚本
    ├── export_import_images.sh   # 镜像导出/导入工具
    └── mysql/                    # MySQL 配置
        └── init/                 # 初始化脚本
```

---

## ?? 快速命令

### 最常用的命令

```bash
# 启动所有服务
docker compose up -d

# 查看服务状态
docker compose ps

# 查看日志
docker compose logs -f

# 停止所有服务
docker compose down

# 重启服务
docker compose restart

# 进入容器
docker compose exec central bash
```

---

## ? 常见问题

### Q: 我应该先看哪个文档？

**A:** 按顺序阅读：
1. Docker_环境配置指南.md（安装）
2. Docker_能保存什么.md（理解原理）?
3. Docker_部署指南.md（实战）

### Q: 新人入职需要多久配置环境？

**A:** 
- **传统方式**：2-3 天
- **使用 Docker**：15-20 分钟（参见 [Docker_能保存什么.md](./Docker_能保存什么.md)）

### Q: Docker 能保存数据库数据吗？

**A:** 
- ? **不能直接保存在镜像中**
- ? **可以使用数据卷持久化**
- 详见 [Docker_能保存什么.md - 数据库数据部分](./Docker_能保存什么.md#1-数据库数据)

### Q: 如何分享环境给团队？

**A:** 三种方式：
1. Git（推荐）- 提交 Dockerfile 和 docker-compose.yml
2. Docker Hub - 推送镜像到云端
3. 文件传输 - 导出镜像为 .tar 文件

详见 [Docker_环境共享指南.md](./Docker_环境共享指南.md)

---

## ?? 文档更新

### 最新更新

- **2024-11-25**: 新增 [Docker_能保存什么.md](./Docker_能保存什么.md)
- **2024-11-25**: 创建文档索引
- **2024-11-25**: 所有 Docker 文档移至 `docunment/项目配置与运行/docker/` 目录

### 贡献

发现文档有错误或需要补充？
- 提交 Issue
- 发起 Pull Request
- 联系项目维护者

---

## ?? 开始使用

**立即开始 3 步走：**

```bash
# 1. 安装 Docker Desktop（首次）
# 参考：Docker_环境配置指南.md

# 2. 克隆项目并启动
git clone <项目地址>
cd poor_server_stl
docker compose up -d

# 3. 验证
docker compose ps
```

**就这么简单！** ??

---

## ?? 外部资源

- [Docker 官方文档](https://docs.docker.com/)
- [Docker Hub](https://hub.docker.com/)
- [Docker 中文社区](https://dockerone.com/)
- [Docker 从入门到实践](https://yeasy.gitbook.io/docker_practice/)

---

**最后更新：** 2024-11-25
