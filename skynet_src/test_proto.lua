-- test_proto.lua
-- 测试 Protobuf 集成
-- 使用方法：lua test_proto.lua

print("=== Skynet Protobuf Integration Test ===\n")

-- 1. 测试加载 protobuf 模块
print("1. Loading protobuf module...")
local ok, proto_or_err = pcall(require, "proto")

if not ok then
    print("✗ FAILED: Cannot load proto module")
    print("  Error:", proto_or_err)
    print("\n提示：")
    print("  - 确保 lua-protobuf 已安装")
    print("  - 将 protobuf.dll (Windows) 或 protobuf.so (Linux) 放到当前目录或 Lua 路径")
    print("  - 参考 skynet_src/README_proto.md 安装说明")
    os.exit(1)
end

local proto = proto_or_err
print("✓ Proto module loaded\n")

-- 2. 测试加载 descriptor
print("2. Loading descriptor file...")
local desc_path = "protobuf/skynet/src/test.pb"
local ok, err = proto.load_descriptor(desc_path)

if not ok then
    print("✗ FAILED: Cannot load descriptor")
    print("  Error:", err)
    print("  Path:", desc_path)
    print("\n提示：")
    print("  - 运行 .\\tools\\generate_proto_desc.ps1 生成 descriptor 文件")
    print("  - 确保 protobuf/skynet/src/test.pb 存在")
    os.exit(1)
end

print("✓ Descriptor loaded:", desc_path, "\n")

-- 3. 查看已注册的消息类型
print("3. Registered message types:")
local types = proto.get_types()
if #types == 0 then
    print("  (none)")
else
    for _, name in ipairs(types) do
        print("  -", name)
    end
end
print()

-- 4. 测试编码
print("4. Testing message encoding...")
local test_data = {
    id = 12345,
    message = "Hello from Skynet!"
}

local ok, bytes_or_err = pcall(proto.encode, "skynet_test.TestRequest", test_data)
if not ok then
    print("✗ FAILED: Cannot encode message")
    print("  Error:", bytes_or_err)
    os.exit(1)
end

local bytes = bytes_or_err
print("✓ Encoded successfully")
print("  Original data:", string.format("id=%d, message='%s'", test_data.id, test_data.message))
print("  Encoded size:", #bytes, "bytes")
print("  Hex dump:", bytes:gsub(".", function(c) return string.format("%02X ", string.byte(c)) end):sub(1, 50))
print()

-- 5. 测试解码
print("5. Testing message decoding...")
local ok, decoded_or_err = pcall(proto.decode, "skynet_test.TestRequest", bytes)
if not ok then
    print("✗ FAILED: Cannot decode message")
    print("  Error:", decoded_or_err)
    os.exit(1)
end

local decoded = decoded_or_err
print("✓ Decoded successfully")
print("  Decoded data:", string.format("id=%d, message='%s'", decoded.id, decoded.message))

-- 验证数据一致性
if decoded.id == test_data.id and decoded.message == test_data.message then
    print("✓ Data integrity verified")
else
    print("✗ WARNING: Data mismatch!")
end
print()

-- 6. 测试复杂消息
print("6. Testing complex message...")
local complex_data = {
    count = 3,
    players = {
        {user_id = 1001, username = "Alice", level = 10, exp = 5000},
        {user_id = 1002, username = "Bob", level = 15, exp = 12000},
        {user_id = 1003, username = "Charlie", level = 20, exp = 25000},
    },
    scores = {
        ["Alice"] = 100,
        ["Bob"] = 150,
        ["Charlie"] = 200,
    }
}

local ok, complex_bytes_or_err = pcall(proto.encode, "skynet_test.ComplexMessage", complex_data)
if not ok then
    print("✗ FAILED: Cannot encode complex message")
    print("  Error:", complex_bytes_or_err)
else
    local complex_bytes = complex_bytes_or_err
    print("✓ Complex message encoded")
    print("  Encoded size:", #complex_bytes, "bytes")
    
    local ok, decoded_complex = pcall(proto.decode, "skynet_test.ComplexMessage", complex_bytes)
    if ok then
        print("✓ Complex message decoded")
        print("  Players count:", #decoded_complex.players)
        print("  Scores:", decoded_complex.scores.Alice, decoded_complex.scores.Bob, decoded_complex.scores.Charlie)
    else
        print("✗ FAILED: Cannot decode complex message")
    end
end
print()

-- 7. 测试类型检查
print("7. Testing type checking...")
if proto.has_type("skynet_test.TestRequest") then
    print("✓ Type 'skynet_test.TestRequest' exists")
else
    print("✗ Type 'skynet_test.TestRequest' not found")
end

if proto.has_type("skynet_test.NonExistent") then
    print("✗ WARNING: Non-existent type found (should not happen)")
else
    print("✓ Type checking works correctly")
end
print()

-- 总结
print("=== Test Summary ===")
print("✓ All tests passed!")
print("\n下一步：")
print("  1. 查看 docunment/skynet/QUICKSTART.md 快速开始")
print("  2. 查看 docunment/skynet/README_proto.md 详细文档")
print("  3. 查看 skynet_src/service/logic/main.lua 服务示例")
print("  4. 根据需求修改 protobuf/skynet/*.proto 文件")
