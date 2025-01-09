-- 网关服务器配置
server = {
    debug = {
        central_server = {
            host = "127.0.0.1",
            port = 50050
        },
        gateway_server = {
            host = "127.0.0.1",
            port = 50051
        },
        db_server = {
            host = "127.0.0.1",
            port = 50052
        },
        login_server = {
            host = "127.0.0.1",
            port = 50053
        },
        logic_server = {
            host = "127.0.0.1",
            port = 50054
        },
        file_server = {
            host = "127.0.0.1",
            port = 50055
        }
    }
    relest = {
    }
}

return server
