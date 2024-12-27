-- 数据库配置
db_config = {
    localDB = {
        poor_users = {
            Host = "127.0.0.1",
            Port = "33060", -- mysqlx协议，所以端口是 33060
            UserName = "root",
            Password = "159357"
        }
    },
    redis = {
        central_server = {
            Host = "127.0.0.1",
            Port = "6379"
        }
    }
}

return db_config
