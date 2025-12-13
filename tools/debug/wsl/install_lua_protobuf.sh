#!/bin/bash
# 安装 lua-protobuf 到 Skynet

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$(dirname "$SCRIPT_DIR")")")"
SKYNET_DIR="$PROJECT_ROOT/skynet_src/skynet"
THIRD_PARTY_DIR="$SKYNET_DIR/3rd"

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

print_header "安装 lua-protobuf 到 Skynet"

# 检查 Skynet 目录
if [ ! -d "$SKYNET_DIR" ]; then
    print_error "Skynet 目录未找到: $SKYNET_DIR"
    exit 1
fi

print_success "Skynet 目录: $SKYNET_DIR"

# 创建 3rd 目录（如果不存在）
mkdir -p "$THIRD_PARTY_DIR"
cd "$THIRD_PARTY_DIR"

# 检查 lua-protobuf 是否已存在
if [ -d "lua-protobuf" ]; then
    print_warning "lua-protobuf 目录已存在"
    read -p "是否删除并重新下载? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_info "删除旧的 lua-protobuf..."
        rm -rf lua-protobuf
    else
        print_info "使用现有的 lua-protobuf 目录"
    fi
fi

# 下载 lua-protobuf
if [ ! -d "lua-protobuf" ]; then
    print_header "下载 lua-protobuf"
    
    print_info "正在从 GitHub 克隆 lua-protobuf..."
    print_info "仓库: https://github.com/starwing/lua-protobuf.git"
    
    if git clone https://github.com/starwing/lua-protobuf.git; then
        print_success "lua-protobuf 下载完成"
    else
        print_error "下载失败"
        print_warning "尝试备用方案：手动下载"
        
        # 备用方案：使用 wget 或 curl 下载 zip
        if command -v wget &> /dev/null; then
            print_info "使用 wget 下载..."
            wget https://github.com/starwing/lua-protobuf/archive/refs/heads/master.zip -O lua-protobuf.zip
            unzip lua-protobuf.zip
            mv lua-protobuf-master lua-protobuf
            rm lua-protobuf.zip
        elif command -v curl &> /dev/null; then
            print_info "使用 curl 下载..."
            curl -L https://github.com/starwing/lua-protobuf/archive/refs/heads/master.zip -o lua-protobuf.zip
            unzip lua-protobuf.zip
            mv lua-protobuf-master lua-protobuf
            rm lua-protobuf.zip
        else
            print_error "无法下载 lua-protobuf，请手动下载并解压到："
            print_error "$THIRD_PARTY_DIR/lua-protobuf"
            exit 1
        fi
    fi
fi

# 编译 lua-protobuf
print_header "编译 lua-protobuf"

cd lua-protobuf

# 检查编译环境
print_info "检查编译环境..."
if ! command -v gcc &> /dev/null; then
    print_error "gcc 未安装，请先安装 gcc"
    print_info "Ubuntu/Debian: sudo apt-get install build-essential"
    exit 1
fi

if ! command -v make &> /dev/null; then
    print_error "make 未安装，请先安装 make"
    exit 1
fi

# 清理旧的编译文件
if [ -f "pb.so" ]; then
    print_info "清理旧的编译文件..."
    make clean || true
fi

# 开始编译
print_info "开始编译 pb.so..."
print_info "编译命令: make"

if make; then
    print_success "编译完成"
else
    print_error "编译失败"
    print_info "尝试手动编译..."
    
    # 手动编译
    gcc -O2 -shared -fPIC pb.c -o pb.so -I/usr/include/lua5.3 -llua5.3
    
    if [ -f "pb.so" ]; then
        print_success "手动编译成功"
    else
        print_error "编译失败，请检查编译环境"
        exit 1
    fi
fi

# 检查 pb.so 是否生成
if [ ! -f "pb.so" ]; then
    print_error "pb.so 未生成"
    exit 1
fi

print_success "pb.so 生成成功"

# 复制到 Skynet luaclib 目录
print_header "安装 pb.so 到 Skynet"

LUACLIB_DIR="$SKYNET_DIR/luaclib"
mkdir -p "$LUACLIB_DIR"

print_info "复制 pb.so 到 $LUACLIB_DIR"
cp pb.so "$LUACLIB_DIR/"

if [ -f "$LUACLIB_DIR/pb.so" ]; then
    print_success "pb.so 安装成功"
    print_info "位置: $LUACLIB_DIR/pb.so"
else
    print_error "pb.so 复制失败"
    exit 1
fi

# 验证安装
print_header "验证安装"

print_info "检查 pb.so..."
if file "$LUACLIB_DIR/pb.so" | grep -q "shared object"; then
    print_success "pb.so 是有效的共享库文件"
else
    print_warning "pb.so 可能不是有效的共享库文件"
fi

# 检查文件大小
FILE_SIZE=$(du -h "$LUACLIB_DIR/pb.so" | cut -f1)
print_info "文件大小: $FILE_SIZE"

# 完成
print_header "安装完成"

print_success "lua-protobuf 已成功安装到 Skynet!"
echo ""
print_info "安装位置:"
echo "  源码: $THIRD_PARTY_DIR/lua-protobuf"
echo "  库文件: $LUACLIB_DIR/pb.so"
echo ""
print_info "下一步:"
echo "  1. 运行 Skynet 测试: ./tools/debug/wsl/start_skynet.sh"
echo "  2. 或手动测试: cd skynet_src/skynet && ./skynet ../../config/skynet/config.lua"
echo ""
print_success "可以开始使用 Protobuf 了！"
