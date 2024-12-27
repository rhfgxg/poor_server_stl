# 各服务器模块功能
## 中心服务器
使用多中心服务器（暂时使用单中心服务器，但开发时应预先为多中心服务器考虑）
主要负责管理所有服务器，包括：服务器启停，活动，服务器之间的通信，监控等，还负责全服消息的发送
所有服务器的活动日志，其他服务器还会有自己的日志
守护其他服务器：中心服务器会定时检查其他服务器的状态，如果发现服务器异常，会自动重启服务器

心跳：中心服务器设立接口，其他服务器定时向中心服务器发送心跳包，中心服务器返回心跳包
中心服务器之间也会有心跳包

中心服务器之间定时同步连接池和心跳数据

管理连接池：所有服务器使用连接池管理与其他服务器的连接
	中心服务器管理所有服务器的连接池：每个类型的服务器，都有一个连接池（暂时中心服务器只有一个，后续进行扩展）
	其他服务器保存所需的链接池备份
	其他服务器定时向中心服务器请求最新的连接池，并进行更新
	其他服务器轮询各自的连接池，与其他服务器通信

## 网关服务器
处理客户端链接，断开
客户端请求通过统一的转发接口，传递服务类型与负载数据包（具体信息）
根据服务类型，将负载反序列为对应请求数据包，然后转发给对应的服务器

## 逻辑服务器
负责处理所有游戏逻辑，包括：战斗，任务，商城等
用户管理：用户数据更新，用户数据存储，用户数据查询等

## 登录服务器
负责处理登录，注册等请求
服务器用户数量管理

## 数据库服务器
负责处理其他服务器对数据库的操作，还负责数据库的备份
+ 处理数据库的增删改查
	- 其他服务器通过指定数据库名，表名，字段名，条件等，向数据库服务器发送请求，数据库服务器处理后返回数据
	- 其他服务器获取数据后，在自己的服务器中对数据进行逻辑处理

+ 数据库的备份
	- 定时备份数据库到另一个服务器

+ 启动时保持数据库的完整性
	- 数据缺失时，报出异常，停止服务器启动，等待管理员使用备份数据恢复数据库
	- 自动创建缺失表
	- 自动更新表字段

# 流程
服务器启动流程：
中心服务器启动
其他服务器自行启动，然后向中心服务器注册

服务器守护：
中心服务器向其他服务器发送心跳包，其他服务器返回心跳包
如果发现服务器异常，会自动重启服务器

客户端请求处理：
客户端所有类型的请求，通过客户端统一的接口，传递服务类型与负载数据包
网关服务器根据服务类型，将负载数据包反序列化为对应的请求数据包，然后转发给对应的服务器
服务器处理请求后，返回数据包给网关服务器
网关服务器将数据包序列化后，返回给客户端

特殊处理文件类型服务：
文件上传下载通过特殊接口进行处理：
客户端请求文件上传下载，网关服务器将请求转发给文件服务器
文件服务器处理文件上传下载请求，返回结果（包含文件服务器地址信息）给网关服务器
网关服务器将结果返回给客户端
客户端根据文件服务器地址信息，直接与文件服务器进行文件上传下载