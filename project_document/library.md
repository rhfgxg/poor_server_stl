# 在项目中使用引入库
除了类似 mysql这种只有一个服务器需要用的库，其他库都放在顶级 CMakeLists.txt文件中引入

# 项目中使用的第三方库
所有第三方库文件都放在 vcpkg_installed/x64-windwos 文件夹下（需要使用vcpkg安装后，自动生成）
第三方库的 debug模式的文件，放在 vcpkg_installed/x64-windwos/debug 文件夹下

## 第三方库列表
仅包含直接使用的库列表

### 网络库
boost库

### 线程库
thread 库：标准库

### grpc：客户端与服务器，服务器之间的通信协议
grpc库
实现服务器之间的远程调用 
    
### mysql库
mysql-connector-cpp库
使用mysqlx协议，所以链接数据库的端口是33060

### redis：缓存库
cpp-redis库
tacopie库
redis在windows下构建时，会导致一些异常，所以需要使用类进行包装，然后使用类的方法来操作redis（详见 central_server.h）

### json操作
nlohmann-json库

### lua
lua 库
使用lua作为配置文件

### protobuf：客户端与服务器，服务器之间的数据序列化
protobuf库：包含在 grpc库中

生成文件用到 protoc编译器，这个文件在 /vcpkg_installed/x64-windows/tools/protobuf/protoc.exe
生成 rpc服务文件时，需要用到 grpc_cpp_plugin.exe，这个文件需要先安装 grpc库，安装后保存在 /vcpkg_installed/x64-windows/tools/grpc/grpc_cpp_plugin.exe

我这里使用 which指令寻找 grpc_cpp_plugin工具时，一直找不到，所以使用的是相对路径
proto 中，string默认是 utf-8格式，与c++冲突，所以在编写 proto文件时，需要使用 bytes代替string

### 日志库
spdlog库

### token 库
jwt-cpp 库

### RSA加密库
openssl库
非对称加密