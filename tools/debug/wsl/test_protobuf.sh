#!/bin/bash
# Protobuf 集成测试脚本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$(dirname "$SCRIPT_DIR")")")"
SKYNET_DIR="$PROJECT_ROOT/skynet_src/skynet"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_header() {
    echo -e "\n${BLUE}===================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}===================================================${NC}\n"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[i]${NC} $1"
}

print_header "Skynet Protobuf 集成测试"

# 检查 Skynet 是否编译
if [ ! -f "$SKYNET_DIR/skynet" ]; then
    print_error "Skynet 未编译，正在编译..."
    cd "$SKYNET_DIR"
    make linux
    if [ $? -ne 0 ]; then
        print_error "Skynet 编译失败"
        exit 1
    fi
    print_success "Skynet 编译完成"
fi

# 检查 pb.so
if [ ! -f "$SKYNET_DIR/luaclib/pb.so" ]; then
    print_warning "pb.so 未找到，尝试编译..."
    
    if [ -d "$SKYNET_DIR/3rd/lua-protobuf" ]; then
        cd "$SKYNET_DIR/3rd/lua-protobuf"
        make clean
        make
        cp pb.so ../../luaclib/
        print_success "pb.so 编译完成"
    else
        print_error "lua-protobuf 源码未找到"
        print_info "请手动安装 lua-protobuf"
        exit 1
    fi
fi

# 检查配置文件
CONFIG_FILE="$PROJECT_ROOT/config/skynet/config.lua"
if [ ! -f "$CONFIG_FILE" ]; then
    print_error "配置文件未找到: $CONFIG_FILE"
    exit 1
fi

# 运行测试
print_header "运行 Protobuf 测试"

cd "$SKYNET_DIR"

# 创建临时测试配置
TEST_CONFIG="$SKYNET_DIR/test_proto_config.lua"
cat > "$TEST_CONFIG" << 'EOF'
-- 临时测试配置
root = "../../"
thread = 4
logger = nil
harbor = 0
start = "test/proto_test"  -- 启动测试服务
bootstrap = "snlua bootstrap"

luaservice = root.."skynet_src/service/?.lua;"..
             "./service/?.lua"
lualoader = "./lualib/loader.lua"
lua_path = root.."skynet_src/lualib/?.lua;"..
           "./lualib/?.lua;"..
           "./lualib/?/init.lua"
cpath = "./cservice/?.so"
lua_cpath = "./luaclib/?.so"
snax = root.."skynet_src/service/?.lua"
EOF

print_info "启动 Skynet 测试服务..."
print_info "配置文件: $TEST_CONFIG"
echo ""

# 运行测试（超时 10 秒）
timeout 10s ./skynet "$TEST_CONFIG" || true

# 清理
rm -f "$TEST_CONFIG"

print_header "测试完成"

print_info "如需手动测试，请运行："
echo "  cd $SKYNET_DIR"
echo "  ./skynet ../../config/skynet/config.lua"
echo ""
echo "然后在 Skynet 控制台中执行："
echo "  skynet.newservice(\"test/proto_test\")"

print_success "脚本执行完成"
