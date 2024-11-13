# poor_server_stl
c++（无qt库）的服务端版本

本项目用来尝试使用c++标准库重构qt版本的服务端
原项目使用 QT库，在面试和实际工作中发现，很少会使用 QT库作为服务器框架
所以决定使用本项目重构原代码，实现并学习下面功能：
1. 分布式服务器，多线程
2. 服务器之间通信和调用：RPC
3. 客户端与服务器，服务器之间通信协议：protobuf
4. 使用分层架构，多模块设计：各模块独享一个 CMakeLists文件，通过顶级 CMakeLists文件管理
5. 使用对象池管理项目中需要频繁创建删除的对象
6. 结合lua语言实现配置文件等的热更新

1. 
# 项目中使用到的知识点
## RPC
RPC：服务器之间的功能调用，像调用本地项目一样调用其他服务器的功能接口

## protobuf
通信协议，用来序列化数据

## mysql
使用 mysql-connector-cpp库，使用mysqlx协议，所以链接数据库的端口是33060


# 项目中使用的第三方库
## 所有第三方库都放在 vcpkg_installed/x64-windwos 文件夹下

# 第三方库列表
仅包含直接使用的库列表

boost：网络库
mysql-connector-cpp：mysql库
nlohmann-json：json操作库
protobuf：protobuf库：客户端与服务器，服务器之间的通信协议