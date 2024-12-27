-- 网关服务器配置
gateway_config = {
    central_server = {
        name = "中心服务器",
        host = "localhost",
        port = 50050
    },
    gateway_server = {
        name = "网关服务器",
        host = "localhost",
        port = 50051
    },
    data_server = {
        name = "数据库服务器",
        db_name = "users",
        host = "localhost",
        port = 50052
    },
    login_server = {
        name = "登录服务器",
        host = "localhost",
        port = 50053
    },
    logic_server = {
        name = "逻辑服务器",
        host = "localhost",
        port = 50054
    }
}

return gateway_config