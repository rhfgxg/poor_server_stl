-- proto.lua
-- Protobuf 辅助封装，依赖 lua-protobuf (https://github.com/starwing/lua-protobuf)
-- 使用方法：
--   local proto = require "proto"
--   proto.load_descriptor("protobuf/desc/server_logic.pb")
--   local bytes = proto.encode("logic.LoginRequest", {user_id = 123, token = "abc"})
--   local msg = proto.decode("logic.LoginResponse", response_bytes)

local M = {}

local ok, pb = pcall(require, "protobuf")
if not ok then
    error([[
lua-protobuf not found!

请安装 lua-protobuf 并将其放置到 skynet_src/luaclib/ 目录:
1. 下载 https://github.com/starwing/lua-protobuf
2. 编译生成 protobuf.so (Linux) 或 protobuf.dll (Windows)
3. 将文件复制到 skynet_src/luaclib/
4. 或者通过 luarocks 安装: luarocks install lua-protobuf

]])
end

-- 已加载的 descriptor 文件缓存
local loaded_files = {}

-- 加载 protobuf descriptor 文件（由 protoc --descriptor_set_out 生成的 .pb 文件）
-- @param path string descriptor 文件路径，相对于项目根目录
-- @return boolean, string 成功返回 true，失败返回 nil 和错误信息
function M.load_descriptor(path)
    if loaded_files[path] then
        return true
    end
    
    local f = io.open(path, "rb")
    if not f then
        return nil, "Cannot open descriptor file: " .. path
    end
    
    local content = f:read("*a")
    f:close()
    
    local ok, err = pcall(function()
        pb.load(content)
    end)
    
    if not ok then
        return nil, "Failed to load descriptor: " .. tostring(err)
    end
    
    loaded_files[path] = true
    return true
end

-- 编码消息：table -> bytes
-- @param msg_name string 消息类型全名，如 "logic.LoginRequest"
-- @param data table 要编码的数据
-- @return string 编码后的二进制数据
function M.encode(msg_name, data)
    local ok, result = pcall(pb.encode, msg_name, data)
    if not ok then
        error(string.format("Failed to encode message '%s': %s", msg_name, tostring(result)))
    end
    return result
end

-- 解码消息：bytes -> table
-- @param msg_name string 消息类型全名，如 "logic.LoginResponse"
-- @param bytes string 要解码的二进制数据
-- @return table 解码后的数据
function M.decode(msg_name, bytes)
    local ok, result = pcall(pb.decode, msg_name, bytes)
    if not ok then
        error(string.format("Failed to decode message '%s': %s", msg_name, tostring(result)))
    end
    return result
end

-- 获取所有已注册的消息类型
-- @return table 消息类型列表
function M.get_types()
    return pb.types() or {}
end

-- 检查消息类型是否存在
-- @param msg_name string 消息类型全名
-- @return boolean 存在返回 true
function M.has_type(msg_name)
    local types = M.get_types()
    for _, name in ipairs(types) do
        if name == msg_name then
            return true
        end
    end
    return false
end

-- 清空所有已加载的 descriptor
function M.clear()
    pb.clear()
    loaded_files = {}
end

return M
