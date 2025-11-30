-- 鏁版嵁搴撳垪琛?
local db_config = {

    ["mysqlx"] = {  -- mysql閰嶇疆琛?
        ["poor_users"] = {  -- 骞冲彴鐢ㄦ埛鏁版嵁搴?
            ["host"] = "127.0.0.1",
            ["port"] = "33060", -- mysqlx鍗忚锛屾墍浠ョ鍙ｆ槸 33060
            ["user_name"] = "root",
            ["password"] = "159357",
            ["tables"] = {
                [1] = "poor_users",    -- 鐢ㄦ埛琛?
            }
        },

        ["poor_hearthstone"] = { -- 鐐夌煶浼犺妯″潡鏁版嵁搴?
            ["host"] = "127.0.0.1",
            ["port"] = "33060",
            ["user_name"] = "root",
            ["password"] = "159357",
            ["tables"] = {
                [1] = "playerdata",   -- 鐜╁鏁版嵁
                [2] = "player_achievements_quests",   -- 鎴愬氨锛屼换鍔?
                [3] = "player_class_wins",    -- 鑱屼笟鑳滃満
                [4] = "player_collection",    -- 鏀惰棌锛氬崱鐗岋紝鐨偆
                [5] = "player_friends",   -- 濂藉弸
            }
        },

        ["poor_net_disk"] = {  -- 缃戠洏鏁版嵁搴?
            ["host"] = "127.0.0.1",
            ["port"] = "33060",
            ["user_name"] = "root",
            ["password"] = "159357",
            ["tables"] = {
                [1] = "user_info",    -- 缃戠洏鐢ㄦ埛琛?
                -- 涓烘瘡涓敤鎴峰缓绔嬩竴寮犳枃浠舵暟鎹〃锛岃〃鍚嶆牸寮忥細file_鐢ㄦ埛鍞竴ID
            }
        }
    },

    ["redis"] = {   -- redis 閰嶇疆琛?
        ["host"] = "127.0.0.1",
        ["port"] = "6379",
    }
}

return db_config
