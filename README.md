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


# 第三方库
所有第三方库文件都放在 vcpkg_installed/x64-windwos 文件夹下
debug版本的库文件放在 vcpkg_installed/x64-windwos/debug 文件夹下

仅包含直接使用的库列表：
_ boost：网络库
_ mysql-connector-cpp：mysql库
_ nlohmann-json：json操作库
_ protobuf：protobuf库：客户端与服务器，服务器之间的通信协议

## mysql
使用 mysql-connector-cpp库，使用mysqlx协议，所以链接数据库的端口是33060
### 安装报错
使用vcpkg安装时，可能会有一个openssl补丁无法下载
	可以修改镜像库配置，不使用镜像库，直接使用github进行下载
debug 运行时，可能会有一个 mysqlcppconn8-2-vs14.dll 异常
	可以使用 vcpkg_installed/x64-windows/debug 下的dll文件

## protobuf
通信协议，用来序列化数据

## RPC
RPC：服务器之间的功能调用，像调用本地项目一样调用其他服务器的功能接口
