#!/bin/bash
# Skynet 服务启动脚本（统一版本）
# 功能：检查环境 -> 编译 Skynet -> 启动服务
# 替代: quick_start.sh, start_skynet_server.sh, test_protobuf.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
SKYNET_DIR="$PROJECT_ROOT/skynet_src/skynet"
CONFIG_FILE="$PROJECT_ROOT/config/skynet/config.lua"

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

print_success() { echo -e "${GREEN}[✓]${NC} $1"; }
print_error() { echo -e "${RED}[✗]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[!]${NC} $1"; }
print_info() { echo -e "${BLUE}[i]${NC} $1"; }

print_header "Skynet 服务启动"

# 步骤 1: 检查 Skynet
print_info "检查 Skynet..."
if [ ! -f "$SKYNET_DIR/skynet" ]; then
    print_warning "Skynet 未编译，正在编译..."
    cd "$SKYNET_DIR"
    make linux
    if [ $? -ne 0 ]; then
        print_error "Skynet 编译失败"
        exit 1
    fi
    print_success "Skynet 编译完成"
else
    print_success "Skynet 已编译"
fi

# 步骤 2: 检查 pb.so（lua-protobuf）
print_info "检查 Protobuf 支持..."
if [ ! -f "$SKYNET_DIR/luaclib/pb.so" ]; then
    print_warning "pb.so 未找到"
    
    if [ -d "$SKYNET_DIR/3rd/lua-protobuf" ]; then
        print_info "发现 lua-protobuf 源码，尝试编译..."
        cd "$SKYNET_DIR/3rd/lua-protobuf"
        make clean || true
        if make; then
            cp pb.so ../../luaclib/
            print_success "pb.so 编译完成"
        else
            print_warning "pb.so 编译失败，将使用降级方案"
        fi
    else
        print_warning "lua-protobuf 源码未找到"
        print_info "系统将使用纯 Lua 序列化（降级方案）"
        print_info "如需最佳性能，请运行: bash $PROJECT_ROOT/tools/service/install_lua_protobuf.sh"
    fi
fi

if [ -f "$SKYNET_DIR/luaclib/pb.so" ]; then
    print_success "Protobuf: 原生库（最优性能）"
else
    print_warning "Protobuf: 纯 Lua 实现（降级模式）"
fi

# 步骤 3: 检查配置文件
if [ ! -f "$CONFIG_FILE" ]; then
    print_error "配置文件未找到: $CONFIG_FILE"
    exit 1
fi
print_success "配置文件: $CONFIG_FILE"

# 步骤 4: 显示启动信息
print_header "启动信息"
print_info "项目根目录: $PROJECT_ROOT"
print_info "Skynet 目录: $SKYNET_DIR"
print_info "配置文件: $CONFIG_FILE"
print_info ""
print_info "服务列表:"
echo "  - C++ Gateway (端口 8888)"
echo "  - Player Manager"
echo "  - Achievement Service"
echo "  - Hearthstone Manager"
echo ""

# 步骤 5: 启动 Skynet
read -p "按 Enter 启动 Skynet（Ctrl+C 取消）..." -r
echo ""

cd "$SKYNET_DIR"
print_header "Skynet 启动中..."
print_warning "按 Ctrl+C 停止服务器"
echo ""

./skynet "$CONFIG_FILE"

echo -e "\n${BLUE}Skynet 已停止${NC}"
