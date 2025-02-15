# poor_server_stl 项目介绍
本项目主要使用c++ 实现一个服务器框架
可能会混有部分lua脚本，实现配置文件的热更新
在 tools 文件夹下，会有一些工具脚本，用来辅助开发

## 项目目的：实现并学习下面功能：
1. 架构设计：
1.1 多模块，分布式服务器架构设计
1.2 多中心服务器设计
1.3 多数据库设计
1.4 服务器间通信与调用：gRPC
1.5 通用数据通信协议：protobuf
1.6 服务器链接，数据库链接管理：连接池
1.7 多线程，异步任务
1.8 日志系统
1.9 使用lua作为配置文件
1.10 主要多服务，多数据库使用主从架构
1.11 事件机制

2. 功能实现：
2.1 游戏：炉石传说
2.2 网盘：文件上传下载，断点续传
2.3 网盘：图片预览，压缩等


# 安装说明
## 源码编译
```git clone https://github.com/rhfgxg/poor_server_stl.git```

## 安装包（暂无）


# 参与项目

## 项目规范
(项目开发文档)[project_document]

## 开发环境建议
### 操作系统
windows 10 / 11
linux（待验证）

### 编译器
#### Visual Studio 2022 配置
1. 使用 VS 参与项目时，可以先下载下面的插件
    FileEncoding
    force utf-8 （no BOM）
用来设置文件编码为 utf-8 无 BOM
2. 使用配置文件 .editorconfig 来设置默认文件编码格式
3. CMakeLists 文件添加编译选项
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
projrct_document/：项目文档
protobuf/：各模块 grpc通信 proto协议文件
src/：各模块源码文件
tools/：工具文件
vpckg_installed/：第三方库文件安装目录

### 参与此项目，你可能需要修改的部分文件
主要包含部分本地环境的配置修改
注意，这些文件请不要提交到 git 仓库中，只用来本地开发              
1. config/server_config.lua    # 服务器配置文件，可以根据需要修改，其余地方的配置会根据这个文件自动修改
2. tools/debug/protoc_make.cmd      # 生成 proto 文件的脚本，根据自己的项目目录修改 PROTOC 和 GRPC_PLUGIN变量，在9，10行


## 编译与部署
### 编译
在本地环境下，使用 cmake 编译项目
得到可执行文件，dll文件

### 部署（脚本一键部署）
部署时，/config/server_config.lua文件会被脚本自动分割（每个服务器独立的配置文件）
为了保证代码一致性，开发时，需要使用另一个脚本，将debug模式的服务器配置文件分割并复制到各个服务器目录下