-- 数据库与redis配置
local db_config = {

    ["mysqlx"] = {  -- mysql配置
        ["poor_users"] = {  -- 用户数据库
            ["host"] = "10.255.255.254",  -- Windows主机IP（从WSL访问）
            ["port"] = "33060", -- mysqlx 只能监听33060端口
            ["user_name"] = "root",
            ["password"] = "159357",
            ["tables"] = {
                [1] = "poor_users",    -- 总用户表
            }
        },

        ["poor_hearthstone"] = { -- 炉石传说数据库
            ["host"] = "10.255.255.254",
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

        ["poor_net_disk"] = {  -- 网盘数据库
            ["host"] = "10.255.255.254",
            ["port"] = "33060",
            ["user_name"] = "root",
            ["password"] = "159357",
            ["tables"] = {
                [1] = "user_info",    -- 网盘用户表
                -- 为每个用户建立一张文件数据表，表名格式：file_用户唯一ID
            }
        },

        ["poor_file_hub"] = {  -- 文件中心数据库
            ["host"] = "10.255.255.254",
            ["port"] = "33060",
            ["user_name"] = "root",
            ["password"] = "159357",
            ["tables"] = {
                -- 动态创建的用户文件表：file_{account}
            }
        }
    },

    ["redis"] = {   -- redis 配置表
        ["host"] = "127.0.0.1",  -- Redis在WSL内运行，使用localhost
        ["port"] = "6379",
    }
}

return db_config
