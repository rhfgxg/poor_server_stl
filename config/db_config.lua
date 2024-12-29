-- 数据库配置
db_config = {
    mysqlx = {
        Host = "127.0.0.1",
        Port = "33060", -- mysqlx协议，所以端口是 33060
        UserName = "root",
        Password = "159357",
        db_name = {
            user_db = "poor_users"
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
