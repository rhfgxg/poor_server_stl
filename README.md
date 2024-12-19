# poor_server_stl
本项目主要使用c++ 实现一个服务器框架
可能会混有部分lua脚本，实现配置文件的热更新
在 tools 文件夹下，会有一些工具脚本，用来辅助开发

项目目的：实现并学习下面功能：
1. 架构设计：
1.1 分布式，多模块服务器设计
1.2 服务器之间通信和调用：RPC
1.3 客户端与服务器，服务器之间通信协议：protobuf
1.4 服务器之间链接管理：链接池
1.5 使用对象池管理项目中需要频繁创建删除的对象
2. 功能实现：
2.1 结合lua语言实现配置文件等的热更新
2.2. 文件上传下载：文件切块，断点续传，文件压缩等
2.3. 日志系统

# 安装及使用说明
## clone 项目
### 
```git clone https://github.com/rhfgxg/poor_server_stl.git```

### 项目依赖及安装
参考：[项目中使用的第三方库](project_document/library.md)


# 文件树
tree.txt：文件树（包含其他文件的详细介绍）
vcpkg_json, vcpkg-configuration.json：第三方库文件列表

config/：系统配置文件
protobuf/：各模块 grpc通信 proto协议文件
src/：各模块源码文件
projrct_document/：项目文档
tools/：工具文件
vpckg_installed/：第三方库文件

