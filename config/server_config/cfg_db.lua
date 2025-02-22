-- 数据库列表
db_config = {
    mysqlx = {
        Host = "127.0.0.1",
        Port = "33060", -- mysqlx协议，所以端口是 33060
        UserName = "root",
        Password = "159357",
        poor_users = {  -- 平台用户总表
            "poor_users"    -- 用户表
        },
        hearthstone = { -- 炉石传说模块表
            "playerdata",   -- 玩家数据
            "player_achievements_quests",   -- 成就，任务
            "player_class_wins",    -- 职业胜场
            "player_collection",    -- 收藏：卡牌，皮肤
            "player_friends",   -- 好友
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
