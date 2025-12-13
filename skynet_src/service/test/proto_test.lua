-- Proto 测试服务
-- 用于验证 Protobuf 编解码是否正常工作

local skynet = require "skynet"
local proto_util = require "proto_util"

skynet.start(function()
    skynet.error("============================================")
    skynet.error("Starting Protobuf Test...")
    skynet.error("============================================")
    
    -- 测试1: 加载 Proto 定义
    skynet.error("\n[Test 1] Loading Proto Definitions...")
    local ok, err = pcall(proto_util.load_proto)
    if not ok then
        skynet.error("FAILED to load proto:", err)
        return
    end
    skynet.error("PASSED: Proto loaded successfully")
    
    -- 测试2: EnterGameRequest 编解码
    skynet.error("\n[Test 2] EnterGameRequest Encode/Decode...")
    local enter_req = {
        player_id = "test_player_12345",
        token = "test_token_abc123",
        client_version = "1.0.0"
    }
    
    local binary, err = proto_util.encode(proto_util.MessageType.MSG_ENTER_GAME, enter_req)
    if not binary then
        skynet.error("FAILED to encode:", err)
        return
    end
    skynet.error("Encoded binary length:", #binary)
    
    local decoded, err = proto_util.decode(proto_util.MessageType.MSG_ENTER_GAME, binary)
    if not decoded then
        skynet.error("FAILED to decode:", err)
        return
    end
    
    skynet.error("Decoded data:")
    proto_util.print_message(proto_util.MessageType.MSG_ENTER_GAME, decoded)
    
    if decoded.player_id == enter_req.player_id and
       decoded.token == enter_req.token and
       decoded.client_version == enter_req.client_version then
        skynet.error("PASSED: Data matches")
    else
        skynet.error("FAILED: Data mismatch")
        return
    end
    
    -- 测试3: EnterGameResponse 编解码
    skynet.error("\n[Test 3] EnterGameResponse Encode/Decode...")
    local enter_res = {
        success = true,
        message = "Welcome to the game!",
        player_data = {
            player_id = "test_player_12345",
            level = 10,
            exp = 2500,
            gold = 5000,
            owned_cards = {"CARD_001", "CARD_002", "CARD_003"},
            achievements = {
                progress = {
                    achievement_1 = 50,
                    achievement_2 = 100
                },
                completed = {
                    achievement_2 = true
                }
            },
            last_login = 1704067200  -- 2024-01-01 00:00:00
        }
    }
    
    local binary, err = proto_util.encode(proto_util.MessageType.MSG_ENTER_GAME_RES, enter_res)
    if not binary then
        skynet.error("FAILED to encode:", err)
        return
    end
    skynet.error("Encoded binary length:", #binary)
    
    local decoded, err = proto_util.decode(proto_util.MessageType.MSG_ENTER_GAME_RES, binary)
    if not decoded then
        skynet.error("FAILED to decode:", err)
        return
    end
    
    skynet.error("Decoded data:")
    proto_util.print_message(proto_util.MessageType.MSG_ENTER_GAME_RES, decoded)
    
    if decoded.success == enter_res.success and
       decoded.message == enter_res.message and
       decoded.player_data.player_id == enter_res.player_data.player_id and
       decoded.player_data.level == enter_res.player_data.level then
        skynet.error("PASSED: Data matches")
    else
        skynet.error("FAILED: Data mismatch")
        return
    end
    
    -- 测试4: PlayerActionRequest 编解码
    skynet.error("\n[Test 4] PlayerActionRequest Encode/Decode...")
    local action_req = {
        player_id = "test_player_12345",
        action_type = "complete_match",
        action_data = string.pack("I4", 12345)  -- 示例：打包一个整数
    }
    
    local binary, err = proto_util.encode(proto_util.MessageType.MSG_PLAYER_ACTION, action_req)
    if not binary then
        skynet.error("FAILED to encode:", err)
        return
    end
    skynet.error("Encoded binary length:", #binary)
    
    local decoded, err = proto_util.decode(proto_util.MessageType.MSG_PLAYER_ACTION, binary)
    if not decoded then
        skynet.error("FAILED to decode:", err)
        return
    end
    
    skynet.error("Decoded data:")
    proto_util.print_message(proto_util.MessageType.MSG_PLAYER_ACTION, decoded)
    skynet.error("PASSED: PlayerActionRequest works")
    
    -- 测试5: 推送消息
    skynet.error("\n[Test 5] Push Messages...")
    local push_achievement = {
        player_id = "test_player_12345",
        achievement = {
            id = "achievement_first_win",
            name = "First Victory",
            description = "Win your first match",
            reward_gold = 100
        }
    }
    
    local binary, err = proto_util.encode(proto_util.MessageType.MSG_PUSH_ACHIEVEMENT, push_achievement)
    if not binary then
        skynet.error("FAILED to encode:", err)
        return
    end
    
    local decoded, err = proto_util.decode(proto_util.MessageType.MSG_PUSH_ACHIEVEMENT, binary)
    if not decoded then
        skynet.error("FAILED to decode:", err)
        return
    end
    
    skynet.error("Decoded push message:")
    proto_util.print_message(proto_util.MessageType.MSG_PUSH_ACHIEVEMENT, decoded)
    skynet.error("PASSED: Push messages work")
    
    -- 测试6: 消息类型工具函数
    skynet.error("\n[Test 6] Message Type Utilities...")
    
    local req_type = proto_util.MessageType.MSG_ENTER_GAME
    local res_type = proto_util.get_response_type(req_type)
    skynet.error(string.format("Request type %d -> Response type %d", req_type, res_type))
    
    if res_type == proto_util.MessageType.MSG_ENTER_GAME_RES then
        skynet.error("PASSED: get_response_type works")
    else
        skynet.error("FAILED: get_response_type failed")
        return
    end
    
    if proto_util.is_response(res_type) then
        skynet.error("PASSED: is_response works")
    else
        skynet.error("FAILED: is_response failed")
        return
    end
    
    if proto_util.is_push(proto_util.MessageType.MSG_PUSH_ACHIEVEMENT) then
        skynet.error("PASSED: is_push works")
    else
        skynet.error("FAILED: is_push failed")
        return
    end
    
    -- 所有测试完成
    skynet.error("\n============================================")
    skynet.error("All Tests PASSED!")
    skynet.error("Protobuf integration is working correctly")
    skynet.error("============================================")
    
    -- 退出测试服务
    skynet.exit()
end)
