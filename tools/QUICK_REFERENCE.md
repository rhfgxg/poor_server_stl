# Tools 脚本快速参考

## 🚀 核心脚本（只需记住这2个）

### 1. 环境安装
```bash
sudo bash tools/setup/wsl_setup.sh
```
**功能**：一键安装所有依赖（基础工具 + Skynet + lua-protobuf + vcpkg）

---

### 2. 服务管理
```bash
bash tools/service/manage.sh <命令>
```

| 命令 | 说明 | 示例 |
|------|------|------|
| `compile` | 编译所有（Proto + Skynet + C++） | `manage.sh compile` |
| `start` | 启动所有服务 | `manage.sh start` |
| `stop` | 停止所有服务 | `manage.sh stop` |
| `restart` | 重启所有服务 | `manage.sh restart` |
| `status` | 查看服务状态 | `manage.sh status` |
| `proto` | 仅生成 Proto 代码 | `manage.sh proto` |
| `skynet` | 仅编译 Skynet | `manage.sh skynet` |
| `cpp` | 仅编译 C++ | `manage.sh cpp` |

---

## 📋 常用流程

### 首次安装
```bash
# 1. 安装环境（含 Skynet）
sudo bash tools/setup/wsl_setup.sh

# 2. 重新加载环境变量
source ~/.bashrc

# 3. 编译项目
bash tools/service/manage.sh compile

# 4. 启动服务
bash tools/service/manage.sh start
```

### 日常开发
```bash
# 编译并重启
bash tools/service/manage.sh compile
bash tools/service/manage.sh restart

# 查看状态
bash tools/service/manage.sh status
```

---

## 🔧 故障排查

### Redis 未运行
```bash
sudo service redis-server start
```

### 查看服务状态
```bash
bash tools/service/manage.sh status
```

### 查看日志
```bash
tail -f logs/*.log
```

---

## 📚 详细文档

- **完整文档**: `tools/README.md`
- **整理总结**: `tools/FINAL_SUMMARY.md`
- **清理计划**: `tools/CLEANUP_PLAN.md`

---

**更新时间**: 2025-12-13  
**快速参考卡片** v1.0
