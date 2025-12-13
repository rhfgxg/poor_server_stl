#!/bin/bash
# Skynet 服务启动脚本（包含 Protobuf 集成）

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$(dirname "$SCRIPT_DIR")")")"
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

print_header "Skynet Logic Server 启动检查"

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
else
    print_success "Skynet 已编译"
fi

# 检查 pb.so（Protobuf 支持）
if [ ! -f "$SKYNET_DIR/luaclib/pb.so" ]; then
    print_warning "pb.so 未找到"
    
    # 检查 lua-protobuf 源码是否存在
    if [ -d "$SKYNET_DIR/3rd/lua-protobuf" ]; then
        print_info "发现 lua-protobuf 源码，尝试编译..."
        cd "$SKYNET_DIR/3rd/lua-protobuf"
        make clean || true
        if make; then
            cp pb.so ../../luaclib/
            print_success "pb.so 编译完成"
        else
            print_error "pb.so 编译失败"
        fi
    else
        print_error "lua-protobuf 源码未找到"
        echo ""
        print_warning "Protobuf 功能需要 lua-protobuf 库"
        print_info "有两种选择："
        echo ""
        echo "  1. 自动安装 lua-protobuf（推荐）"
        echo "     运行: ./tools/debug/wsl/install_lua_protobuf.sh"
        echo ""
        echo "  2. 使用内嵌 Protobuf 定义（性能略低但功能完整）"
        echo "     直接启动即可，系统会自动回退"
        echo ""
        read -p "是否现在自动安装 lua-protobuf? (y/n): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            print_info "运行安装脚本..."
            bash "$SCRIPT_DIR/install_lua_protobuf.sh"
            if [ $? -eq 0 ]; then
                print_success "lua-protobuf 安装成功"
            else
                print_error "安装失败，将使用内嵌定义"
            fi
        else
            print_warning "跳过安装，将使用内嵌 Protobuf 定义"
        fi
    fi
fi

# 最终检查 pb.so
if [ -f "$SKYNET_DIR/luaclib/pb.so" ]; then
    print_success "pb.so 已就绪（使用原生库，性能最优）"
else
    print_warning "pb.so 未找到，将使用内嵌 Protobuf 定义（性能略低）"
fi

# 检查配置文件
if [ ! -f "$CONFIG_FILE" ]; then
    print_error "配置文件未找到: $CONFIG_FILE"
    exit 1
else
    print_success "配置文件: $CONFIG_FILE"
fi

# 检查必要的服务文件
print_info "检查服务文件..."
REQUIRED_SERVICES=(
    "$PROJECT_ROOT/skynet_src/service/main.lua"
    "$PROJECT_ROOT/skynet_src/service/cpp_gateway.lua"
    "$PROJECT_ROOT/skynet_src/service/player_manager.lua"
    "$PROJECT_ROOT/skynet_src/lualib/proto_util.lua"
)

ALL_FOUND=true
for service in "${REQUIRED_SERVICES[@]}"; do
    if [ ! -f "$service" ]; then
        print_error "服务文件未找到: $service"
        ALL_FOUND=false
    fi
done

if [ "$ALL_FOUND" = true ]; then
    print_success "所有必需服务文件已就绪"
else
    print_error "部分服务文件缺失，请检查项目完整性"
    exit 1
fi

# 显示启动信息
print_header "启动 Skynet Logic Server"

print_info "项目根目录: $PROJECT_ROOT"
print_info "Skynet 目录: $SKYNET_DIR"
print_info "配置文件: $CONFIG_FILE"
print_info ""
print_info "服务列表:"
echo "  - C++ Gateway (监听端口 8888)"
echo "  - Player Manager"
echo "  - Achievement Service"
echo "  - Hearthstone Manager"
print_info ""

if [ -f "$SKYNET_DIR/luaclib/pb.so" ]; then
    print_info "Protobuf: ✅ 原生库（最优性能）"
else
    print_info "Protobuf: ⚠️  内嵌定义（降级模式）"
fi
echo ""

# 询问是否启动
read -p "按 Enter 键启动服务器（或 Ctrl+C 取消）..." -r
echo ""

# 切换到 Skynet 目录并启动
cd "$SKYNET_DIR"

print_header "Skynet 启动中..."
print_warning "按 Ctrl+C 停止服务器"
echo ""

# 启动 Skynet
./skynet "$CONFIG_FILE"

# 如果 Skynet 退出
echo ""
print_header "Skynet 已停止"
print_info "查看日志以了解详情"
