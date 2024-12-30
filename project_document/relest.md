# 部署过程
## 配置文件准备
将 /config/server_config.lua 文件根据部署目标进行分割，分割后每个服务器只保留自己相关的配置文件，如：
```
central_server = {
    host = "127.0.0.1",
    port = 50050
}

return central_server
```
根据服务器列表，将配置文件和可执行文件放到对应服务器上