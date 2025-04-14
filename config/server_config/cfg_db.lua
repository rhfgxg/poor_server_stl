-- 数据库列表
local db_config = {

    ["mysqlx"] = {  -- mysql配置表
        ["poor_users"] = {  -- 平台用户数据库
            ["host"] = "127.0.0.1",
            ["port"] = "33060", -- mysqlx协议，所以端口是 33060
            ["user_name"] = "root",
            ["password"] = "159357",
            ["tables"] = {
                [1] = "poor_users",    -- 用户表
            }
        },

        ["hearthstone"] = { -- 炉石传说模块数据库
            ["host"] = "127.0.0.1",
            ["port"] = "33060",
            ["user_name"] = "root",
            ["password"] = "159357",
            ["tables"] = {
                [1] = "playerdata",   -- 玩家数据
                [2] = "player_achievements_quests",   -- 成就，任务
                [3] = "player_class_wins",    -- 职业胜场
                [4] = "player_collection",    -- 收藏：卡牌，皮肤
                [5] = "player_friends",   -- 好友
            }
        },

        ["poor_file_hub"] = {  -- 网盘数据库
            ["host"] = "127.0.0.1",
            ["port"] = "33060",
            ["user_name"] = "root",
            ["password"] = "159357",
            ["tables"] = {
                [1] = "user_info",    -- 网盘用户表
                -- 为每个用户建立一张文件数据表，表名格式：file_用户唯一ID
            }
        }
    },

    ["redis"] = {   -- redis 配置表
        ["host"] = "127.0.0.1",
        ["port"] = "6379",
    }
}

return db_config
