# Skynet Protobuf 集成 - 文档索引

欢迎！本文档帮助你快速找到所需的信息。

## 🚀 我该从哪里开始？

### 第一次使用？
👉 **[快速开始指南](QUICKSTART.md)** - 10分钟上手

### 想深入了解？
👉 **[详细文档](README_proto.md)** - 完整的 API、配置和 FAQ

### 想了解项目结构？
👉 **[项目结构说明](PROJECT_STRUCTURE.md)** - 完整的目录组织



## 📚 文档列表

| 文档 | 说明 | 适合对象 |
|------|------|----------|
| **[QUICKSTART.md](QUICKSTART.md)** | 快速开始，安装和测试 | 新手 |
| **[README_proto.md](README_proto.md)** | Protobuf 详细文档 | 开发者 |
| **[PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md)** | 项目目录结构 | 所有人 |


## 🎯 常见任务

### 安装 lua-protobuf
```powershell
luarocks install lua-protobuf
# 复制 protobuf.dll 到 skynet_src/luaclib/
```
详见：[QUICKSTART.md - 第1步](QUICKSTART.md#1-安装-lua-protobuf)

### 生成 Descriptor 文件
```powershell
.\tools\generate_proto_desc.ps1
```
详见：[QUICKSTART.md - 第2步](QUICKSTART.md#2-生成-protobuf-descriptor-文件)

### 创建新的 Proto 文件
1. 在 `protobuf/skynet/` 创建 `.proto` 文件
2. 运行生成脚本
3. 在 Lua 中加载

详见：[README_proto.md - 使用方法](README_proto.md#三在-skynet-服务中使用-protobuf)

### 开发新服务
参考：
- `skynet_src/service/logic/main.lua` - 逻辑服务器示例
- `skynet_src/service/battle/main.lua` - 战斗服务器示例

详见：[PROJECT_STRUCTURE.md - 工作流程](PROJECT_STRUCTURE.md#工作流程)

### 与 C++ 服务器通信
详见：[README_proto.md - 与 C++ 服务器通信](README_proto.md#2-与-c-服务器通信示例)

## 🔧 其他资源

### Proto 文件说明
👉 `protobuf/skynet/README.md` - Proto 文件的组织和使用

### 代码示例
- `skynet_src/lualib/proto.lua` - Protobuf 辅助库源码
- `skynet_src/test_proto.lua` - 测试脚本，可作为示例
- `skynet_src/service/logic/main.lua` - 完整的服务实现

### 工具脚本
- `tools/generate_proto_desc.ps1` - Windows proto 生成
- `tools/generate_proto_desc.sh` - Linux/macOS proto 生成

## ❓ 遇到问题？

1. **查看 FAQ**: [README_proto.md - 常见问题](README_proto.md#五常见问题)
2. **检查路径**: [PROJECT_STRUCTURE.md - 快速参考](PROJECT_STRUCTURE.md#快速参考)
3. **运行测试**: `lua skynet_src/test_proto.lua`

## 📖 Skynet 学习资源

本目录下还有其他 Skynet 文档：
- `README_skynet.md` - Skynet 框架介绍
- `skynet_quickstart.md` - Skynet 快速入门
- `skynet_concepts.md` - Skynet 核心概念
- `skynet_api_reference.md` - Skynet API 参考

## 🎓 推荐学习路径

1. **了解 Skynet 基础**
   - 阅读 `README_skynet.md`
   - 阅读 `skynet_quickstart.md`

2. **集成 Protobuf**
   - 阅读 `QUICKSTART.md`
   - 运行测试脚本

3. **开发服务**
   - 参考 `service/logic/main.lua`
   - 阅读 `README_proto.md`

4. **生产部署**
   - 阅读 `PROJECT_STRUCTURE.md`
   - 配置和优化

## 📝 文档版本

- **创建日期**: 2025年11月22日
- **最后更新**: 2025年11月22日
- **版本**: 1.0

## 📬 反馈

如有问题或建议，请联系项目维护团队。

---

**提示**: 建议从 [QUICKSTART.md](QUICKSTART.md) 开始！
