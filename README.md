# poor_server_stl 项目介绍
本项目主要使用c++ 实现一个服务器框架
可能会混有部分lua脚本，实现配置文件的热更新
在 tools 文件夹下，会有一些工具脚本，用来辅助开发

## 项目目的：实现并学习下面功能：
1. 架构设计：
- 分布式，主从服务器架构设计
- 多数据库设计
- 服务器间通信与调用：RPC
- 通用数据通信协议：protobuf
- 多线程，异步任务
- 日志系统 
- 使用lua作为配置文件
- 使用 lua (skynet)作为逻辑服务器和对战服务器等，以此实现复杂逻辑实现的简化

2. 功能实现：
- 游戏：炉石传说
- 网盘：文件上传下载，断点续传
- 网盘：图片预览，压缩等

3. 性能优化：
- 使用服务器，数据库连接池，减少连接创建销毁开销
- 使用对象池，减少对象创建销毁开销
- 使用多级缓存（内存，redis，数据库），提升数据访问速度，减少数据库访问压力


# 安装说明
## 源码编译
```
git clone https://github.com/rhfgxg/poor_server_stl.git
```

## 可执行文件（暂无）


# 参与项目

## 项目规范
[项目开发文档](project_document)

## 开发环境建议
### 操作系统
windows 10 / 11
linux（待验证）

### 编译器
#### Visual Studio 2022 配置
1. 使用 VS 参与项目时，建议先下载下面的插件
    FileEncoding
    force utf-8 （no BOM）
用来设置文件编码为 utf-8 无 BOM
2. 使用配置文件 .editorconfig 来设置默认文件编码格式
3. 同时将默认换行符设置为 LF
4. CMakeLists 文件添加编译选项
```cmake
# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
add_compile_options(/wd4828)    # 忽略utf-8相关警告
```

### 第三方库管理工具
vcpkg

### 项目依赖及安装
参考：[项目中使用的第三方库](project_document/library.md)

## 项目文件树，项目关键文件说明
[文件树](tree.txt)

一些关键文件说明：
tree.txt：文件树（包含其他文件的详细介绍）
vcpkg_json：第三方库文件列表
.editorconfig：VS 配置文件，设置文件编码格式为 utf-8（不使用 VS 可以忽略）

config/：系统配置文件
docunment/：项目文档
  └── skynet/：Skynet Protobuf 集成文档（⭐ 新增）
protobuf/：各模块通信 proto协议文件
  ├── cpp/：C++ 服务器 gRPC proto 文件
  └── skynet/：Skynet 服务器 proto 文件（⭐ 新增）
src/：C++ 源码
skynet_src/：Skynet 框架源码，实现逻辑服务器和战斗服务器等
  ├── lualib/proto.lua：Protobuf 辅助库（⭐ 新增）
  └── service/：Skynet 服务（logic, battle）（⭐ 新增）
tools/：工具文件
  ├── generate_proto_desc.ps1：生成 Skynet proto descriptor（⭐ 新增）
  └── generate_proto_desc.sh：Linux 版本（⭐ 新增）
vcpkg_installed/：第三方库文件安装目录

### 参与此项目，你可能需要修改的部分文件
主要包含部分本地环境的配置修改
注意，这些文件请不要提交到 git 仓库中，只用来本地开发              
1. config/server_config.lua    # 服务器配置文件，可以根据需要修改，其余地方的配置会根据这个文件自动修改
2. tools/debug/protoc_make.cmd      # 生成 proto 文件的脚本，根据自己的项目目录修改 PROTOC 和 GRPC_PLUGIN变量，在9，10行


## 运行项目（vs2022）
使用 vs2022 打开项目文件夹

打开终端，运行项目脚本
```
复制配置文件到生成目录
./tools/debug/copy_config.cmd

使用 vcpkg 安装项目所需的第三方库（清单模式，所需库列表在 vcpkg.json 文件中）
vcpkg install

编译 proto 文件
./tools/debug/protoc_make.cmd

生成 Skynet proto descriptor 文件（⭐ 新增）
./tools/generate_proto_desc.ps1

```

## Skynet Protobuf 集成（⭐ 新增）

本项目已集成 Skynet 与 Protobuf，用于实现逻辑服务器和战斗服务器。

### 快速开始
```powershell
# 1. 安装 lua-protobuf
luarocks install lua-protobuf
# 复制 protobuf.dll 到 skynet_src/luaclib/

# 2. 生成 descriptor 文件
.\tools\generate_proto_desc.ps1

# 3. 测试集成
cd skynet_src
lua test_proto.lua
```

### 文档
- **快速开始**: `docunment/skynet/QUICKSTART.md`
- **详细文档**: `docunment/skynet/README_proto.md`
- **项目结构**: `docunment/skynet/PROJECT_STRUCTURE.md`
- **文档索引**: `docunment/skynet/README.md`

### 目录结构
```
protobuf/skynet/          # Skynet proto 文件
├── *.proto               # Proto 定义
└── src/*.pb              # 生成的 descriptor

skynet_src/
├── lualib/proto.lua      # Protobuf 辅助库
├── service/
│   ├── logic/            # 逻辑服务器
│   └── battle/           # 战斗服务器
└── test_proto.lua        # 测试脚本
```

详见：[Skynet 文档索引](docunment/skynet/README.md)

## 编译与部署
### 编译
在本地环境下，使用 cmake 编译项目
得到可执行文件，dll文件

### 部署（脚本一键部署）
部署时，/config/server_config.lua文件会被脚本自动分割（每个服务器独立的配置文件）
为了保证代码一致性，开发时，需要使用另一个脚本，将debug模式的服务器配置文件分割并复制到各个服务器目录下