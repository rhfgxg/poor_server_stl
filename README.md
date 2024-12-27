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

2. 功能实现：
2.1 游戏：炉石传说
2.2 网盘：文件上传下载，断点续传
2.3 网盘：图片预览，压缩等


# 安装说明
## 源码编译
```git clone https://github.com/rhfgxg/poor_server_stl.git```

## 安装包


# 开发环境
建议文件编码全部使用utf-8，以免导致编译错误
cmd脚本使用 unix 格式，以免在 windows 下运行时出现错误

## 操作系统
windows 10 / 11
linux（待验证）

## 编译器
### Visual Studio 2022 配置
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

## 第三方库管理工具
vcpkg

## 项目依赖及安装
参考：[项目中使用的第三方库](project_document/library.md)


# 文件树
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

