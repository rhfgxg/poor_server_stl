--[[
    数据库通用接口模块
    
    提供统一的 CRUD 操作接口，通过 C++ Gateway 转发到 DBServer
    
    使用方式：
        local db = require "db"
        
        -- 初始化（启动时调用一次）
        db.init()
        
        -- 创建记录
        local ok, err = db.create("database_name", "table_name", {
            field1 = "value1",
            field2 = "value2",
        })
        
        -- 读取记录
        local results, err = db.read("database_name", "table_name", {
            id = "123",  -- 查询条件
        })
        
        -- 更新记录
        local ok, err = db.update("database_name", "table_name", 
            { id = "123" },           -- 查询条件
            { field1 = "new_value" }  -- 更新数据
        )
        
        -- 删除记录
        local ok, err = db.delete("database_name", "table_name", {
            id = "123",  -- 查询条件
        })
]]

local skynet = require "skynet"

local M = {}

-- ==================== 状态 ====================

local initialized = false
local gateway_service = nil

-- ==================== Gateway 访问 ====================

local function get_gateway()
    if not gateway_service then
        local ok, gw = pcall(skynet.queryservice, "gateway/cpp_gateway")
        if ok and gw then
            gateway_service = gw
        end
    end
    return gateway_service
end

-- ==================== 初始化 ====================

--- 初始化数据库模块
--- @return boolean 是否成功
function M.init()
    if initialized then
        return true
    end
    
    skynet.error("[DB] Initializing...")
    
    local gw = get_gateway()
    if gw then
        skynet.error("[DB] Connected to gateway/cpp_gateway")
        initialized = true
        return true
    else
        skynet.error("[DB] ERROR: gateway/cpp_gateway not available")
        return false
    end
end

--- 获取模块状态
--- @return table 状态信息
function M.info()
    return {
        initialized = initialized,
        gateway_available = gateway_service ~= nil,
    }
end

-- ==================== CRUD 接口 ====================

--- 创建记录
--- @param database string 数据库名
--- @param table_name string 表名
--- @param data table 要插入的数据 {field = value, ...}
--- @return boolean 是否成功
--- @return string|nil 错误信息
function M.create(database, table_name, data)
    if not initialized then
        M.init()
    end
    
    local gateway = get_gateway()
    if not gateway then
        return false, "gateway not available"
    end
    
    local ok, result = pcall(skynet.call, gateway, "lua", "db_request", {
        action = "create",
        database = database,
        table = table_name,
        data = data,
    })
    
    if not ok then
        return false, tostring(result)
    end
    
    if result then
        return result.success, result.message
    end
    return false, "no response"
end

--- 读取记录
--- @param database string 数据库名
--- @param table_name string 表名
--- @param query table 查询条件 {field = value, ...}
--- @return table|nil 查询结果数组
--- @return string|nil 错误信息
function M.read(database, table_name, query)
    if not initialized then
        M.init()
    end
    
    local gateway = get_gateway()
    if not gateway then
        return nil, "gateway not available"
    end
    
    local ok, result = pcall(skynet.call, gateway, "lua", "db_request", {
        action = "read",
        database = database,
        table = table_name,
        query = query,
    })
    
    if not ok then
        return nil, tostring(result)
    end
    
    if result and result.success then
        local records = {}
        if result.results then
            for _, row in ipairs(result.results) do
                if row.fields then
                    table.insert(records, row.fields)
                else
                    table.insert(records, row)
                end
            end
        end
        return records, nil
    end
    
    return nil, result and result.message or "query failed"
end

--- 更新记录
--- @param database string 数据库名
--- @param table_name string 表名
--- @param query table 查询条件 {field = value, ...}
--- @param data table 要更新的数据 {field = value, ...}
--- @return boolean 是否成功
--- @return string|nil 错误信息
function M.update(database, table_name, query, data)
    if not initialized then
        M.init()
    end
    
    local gateway = get_gateway()
    if not gateway then
        return false, "gateway not available"
    end
    
    local ok, result = pcall(skynet.call, gateway, "lua", "db_request", {
        action = "update",
        database = database,
        table = table_name,
        query = query,
        data = data,
    })
    
    if not ok then
        return false, tostring(result)
    end
    
    if result then
        return result.success, result.message
    end
    return false, "no response"
end

--- 删除记录
--- @param database string 数据库名
--- @param table_name string 表名
--- @param query table 查询条件 {field = value, ...}
--- @return boolean 是否成功
--- @return string|nil 错误信息
function M.delete(database, table_name, query)
    if not initialized then
        M.init()
    end
    
    local gateway = get_gateway()
    if not gateway then
        return false, "gateway not available"
    end
    
    local ok, result = pcall(skynet.call, gateway, "lua", "db_request", {
        action = "delete",
        database = database,
        table = table_name,
        query = query,
    })
    
    if not ok then
        return false, tostring(result)
    end
    
    if result then
        return result.success, result.message
    end
    return false, "no response"
end

-- ==================== 便捷方法 ====================

--- 读取单条记录
--- @param database string 数据库名
--- @param table_name string 表名
--- @param query table 查询条件
--- @return table|nil 单条记录
--- @return string|nil 错误信息
function M.read_one(database, table_name, query)
    local results, err = M.read(database, table_name, query)
    if results and #results > 0 then
        return results[1], nil
    end
    return nil, err or "not found"
end

--- 检查记录是否存在
--- @param database string 数据库名
--- @param table_name string 表名
--- @param query table 查询条件
--- @return boolean 是否存在
function M.exists(database, table_name, query)
    local results, _ = M.read(database, table_name, query)
    return results and #results > 0
end

return M
