-- battle/main.lua
-- 战斗服务器主服务
-- 职责：处理战斗逻辑，战斗房间管理，与逻辑服务器和网关通信

local skynet = require "skynet"
local socket = require "skynet.socket"
local proto = require "proto"

local CMD = {}

-- 配置
local config = {
    logic_host = "127.0.0.1",
    logic_port = 8003,
    tick_interval = 100,  -- 战斗帧更新间隔（毫秒）
}

-- 战斗房间管理
local battle_rooms = {}  -- room_id -> room_info
local next_room_id = 1

-----------------------------------------------------------
-- 战斗房间数据结构
-----------------------------------------------------------

local BattleRoom = {}
BattleRoom.__index = BattleRoom

function BattleRoom.new(room_id, players)
    local room = {
        room_id = room_id,
        players = players or {},  -- {user_id, user_id, ...}
        state = "waiting",        -- waiting, playing, finished
        start_time = nil,
        current_frame = 0,
        actions = {},             -- 每帧的操作记录
    }
    setmetatable(room, BattleRoom)
    return room
end

-- 开始战斗
function BattleRoom:start()
    if self.state ~= "waiting" then
        return false, "Room is not in waiting state"
    end
    
    self.state = "playing"
    self.start_time = skynet.time()
    self.current_frame = 0
    
    skynet.error(string.format("[Battle] Room %d started with %d players", 
        self.room_id, #self.players))
    
    -- 启动战斗循环
    skynet.fork(function()
        self:battle_loop()
    end)
    
    return true
end

-- 战斗主循环
function BattleRoom:battle_loop()
    while self.state == "playing" do
        self.current_frame = self.current_frame + 1
        
        -- 处理当前帧的操作
        self:process_frame()
        
        -- 检查战斗是否结束
        if self:check_battle_end() then
            self:finish()
            break
        end
        
        -- 等待下一帧
        skynet.sleep(config.tick_interval // 10)  -- skynet.sleep 单位是 0.01秒
    end
end

-- 处理当前帧
function BattleRoom:process_frame()
    -- TODO: 实现战斗逻辑
    -- 1. 收集玩家操作
    -- 2. 计算伤害、技能效果
    -- 3. 更新战斗状态
    -- 4. 广播给所有玩家
    
    -- 示例：简单的帧更新日志
    if self.current_frame % 100 == 0 then
        skynet.error(string.format("[Battle] Room %d frame %d", 
            self.room_id, self.current_frame))
    end
end

-- 检查战斗是否结束
function BattleRoom:check_battle_end()
    -- TODO: 实现战斗结束条件判断
    -- 例如：某一方血量为0，达到时间限制等
    
    -- 示例：1000 帧后结束
    return self.current_frame >= 1000
end

-- 结束战斗
function BattleRoom:finish()
    self.state = "finished"
    
    skynet.error(string.format("[Battle] Room %d finished at frame %d", 
        self.room_id, self.current_frame))
    
    -- TODO: 计算战斗结果
    -- TODO: 通知逻辑服务器战斗结果
    -- TODO: 清理房间资源
end

-- 添加玩家操作
function BattleRoom:add_player_action(user_id, action_type, action_data)
    if self.state ~= "playing" then
        return false, "Battle is not playing"
    end
    
    table.insert(self.actions, {
        frame = self.current_frame,
        user_id = user_id,
        action_type = action_type,
        action_data = action_data,
    })
    
    return true
end

-----------------------------------------------------------
-- Protobuf 消息处理
-----------------------------------------------------------

-- 发送 protobuf 消息
local function send_proto_msg(fd, msg_name, data)
    local body = proto.encode(msg_name, data)
    local len = #body
    local header = string.pack(">I4", len)
    socket.write(fd, header .. body)
    skynet.error(string.format("[Battle] Send: %s, size: %d", msg_name, len))
end

-- 接收 protobuf 消息
local function recv_proto_msg(fd, msg_name)
    local header = socket.read(fd, 4)
    if not header then
        return nil
    end
    
    local len = string.unpack(">I4", header)
    if len <= 0 or len > 1024 * 1024 then
        skynet.error(string.format("[Battle] Invalid message length: %d", len))
        return nil
    end
    
    local body = socket.read(fd, len)
    if not body then
        return nil
    end
    
    local ok, msg = pcall(proto.decode, msg_name, body)
    if not ok then
        skynet.error(string.format("[Battle] Failed to decode %s: %s", msg_name, msg))
        return nil
    end
    
    skynet.error(string.format("[Battle] Recv: %s, size: %d", msg_name, len))
    return msg
end

-----------------------------------------------------------
-- 服务命令接口
-----------------------------------------------------------

-- 启动战斗服务器
function CMD.start()
    skynet.error("[Battle] Battle server starting...")
    
    -- 加载 protobuf descriptor
    local ok, err = proto.load_descriptor("protobuf/skynet/src/server_battle.pb")
    if not ok then
        skynet.error("[Battle] Failed to load battle proto:", err)
    end
    
    proto.load_descriptor("protobuf/skynet/src/common.pb")
    
    skynet.error("[Battle] Battle server started")
    return true
end

-- 停止战斗服务器
function CMD.stop()
    skynet.error("[Battle] Battle server stopping...")
    
    -- 结束所有战斗房间
    for room_id, room in pairs(battle_rooms) do
        if room.state == "playing" then
            room:finish()
        end
    end
    
    skynet.error("[Battle] Battle server stopped")
    return true
end

-- 创建战斗房间
-- @param players table 玩家列表 {user_id, user_id, ...}
-- @return number room_id
function CMD.create_room(players)
    local room_id = next_room_id
    next_room_id = next_room_id + 1
    
    local room = BattleRoom.new(room_id, players)
    battle_rooms[room_id] = room
    
    skynet.error(string.format("[Battle] Created room %d with %d players", 
        room_id, #players))
    
    return room_id
end

-- 开始战斗
-- @param room_id number 房间ID
function CMD.start_battle(room_id)
    local room = battle_rooms[room_id]
    if not room then
        return false, "Room not found"
    end
    
    return room:start()
end

-- 处理玩家操作
-- @param room_id number 房间ID
-- @param user_id number 用户ID
-- @param action_type string 操作类型
-- @param action_data table 操作数据
function CMD.player_action(room_id, user_id, action_type, action_data)
    local room = battle_rooms[room_id]
    if not room then
        return false, "Room not found"
    end
    
    return room:add_player_action(user_id, action_type, action_data)
end

-- 获取房间状态
-- @param room_id number 房间ID
function CMD.get_room_state(room_id)
    local room = battle_rooms[room_id]
    if not room then
        return nil, "Room not found"
    end
    
    return {
        room_id = room.room_id,
        state = room.state,
        current_frame = room.current_frame,
        player_count = #room.players,
    }
end

-- 获取所有房间信息
function CMD.get_all_rooms()
    local rooms = {}
    for room_id, room in pairs(battle_rooms) do
        table.insert(rooms, {
            room_id = room.room_id,
            state = room.state,
            current_frame = room.current_frame,
            player_count = #room.players,
        })
    end
    return rooms
end

-- 清理已结束的房间
function CMD.cleanup_finished_rooms()
    local cleaned = 0
    for room_id, room in pairs(battle_rooms) do
        if room.state == "finished" then
            battle_rooms[room_id] = nil
            cleaned = cleaned + 1
        end
    end
    skynet.error(string.format("[Battle] Cleaned %d finished rooms", cleaned))
    return cleaned
end

-----------------------------------------------------------
-- Skynet 服务启动
-----------------------------------------------------------

skynet.start(function()
    -- 注册 lua 协议的消息处理
    skynet.dispatch("lua", function(session, source, command, ...)
        local f = CMD[command]
        if f then
            if session == 0 then
                f(...)
            else
                skynet.ret(skynet.pack(f(...)))
            end
        else
            skynet.error(string.format("[Battle] Unknown command: %s", command))
            if session ~= 0 then
                skynet.ret(skynet.pack(nil))
            end
        end
    end)
    
    -- 自动启动
    CMD.start()
    
    -- 启动定期清理任务
    skynet.fork(function()
        while true do
            skynet.sleep(6000)  -- 每 60 秒清理一次
            CMD.cleanup_finished_rooms()
        end
    end)
end)
