#!/bin/bash
# WSL2 环境一键安装脚本
# 用于自动检查和安装 poor_server_stl 项目所需的所有依赖
# 包括：基础工具、Protobuf、数据库、Lua、vcpkg、Skynet、lua-protobuf

set -e

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

# 检查是否以 root 权限运行
if [ "$EUID" -ne 0 ]; then 
    print_error "请使用 sudo 运行此脚本"
    echo "使用方法: sudo bash setup_wsl2_environment.sh"
    exit 1
fi

# 获取实际用户
REAL_USER=${SUDO_USER:-$USER}
REAL_HOME=$(eval echo ~$REAL_USER)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"

print_header "WSL2 环境一键安装脚本"
echo "项目: poor_server_stl"
echo "当前用户: $REAL_USER"
echo "项目目录: $PROJECT_ROOT"

# 1. 系统信息
print_header "1. 检查系统信息"
print_info "操作系统: $(lsb_release -d | cut -f2)"
print_info "内核版本: $(uname -r)"
print_info "架构: $(uname -m)"

if grep -qi microsoft /proc/version; then
    print_success "检测到 WSL2 环境"
else
    print_warning "未检测到 WSL2 环境，可能是原生 Linux"
fi

# 2. 更新系统软件源
print_header "2. 更新系统软件源"
print_info "正在更新 apt 软件源..."
apt-get update -qq
print_success "软件源更新完成"

# 3. 安装基础开发工具
print_header "3. 安装基础开发工具"

BASIC_TOOLS=(
    "build-essential" "cmake" "ninja-build" "git" "curl" "wget"
    "pkg-config" "autoconf" "automake" "autoconf-archive" "libtool"
    "m4" "make" "unzip" "zip" "tar" "tree"
)

for tool in "${BASIC_TOOLS[@]}"; do
    if dpkg -l | grep -q "^ii  $tool "; then
        print_success "$tool 已安装"
    else
        print_info "正在安装 $tool..."
        apt-get install -y -qq "$tool"
        print_success "$tool 安装完成"
    fi
done

# 4. 安装 Protobuf 编译器
print_header "4. 安装 Protocol Buffers"
if command -v protoc &> /dev/null; then
    PROTOC_VERSION=$(protoc --version | awk '{print $2}')
    print_success "protoc 已安装 (版本: $PROTOC_VERSION)"
else
    print_info "正在安装 protobuf-compiler..."
    apt-get install -y -qq protobuf-compiler libprotobuf-dev
    print_success "protobuf-compiler 安装完成"
fi

# 5. 安装 MySQL 客户端
print_header "5. 安装 MySQL 客户端"
if dpkg -l | grep -q "^ii  mysql-client "; then
    print_success "MySQL 客户端已安装"
else
    print_info "正在安装 MySQL 客户端..."
    apt-get install -y -qq mysql-client libmysqlclient-dev
    print_success "MySQL 客户端安装完成"
fi

# 6. 安装 Redis
print_header "6. 安装 Redis"
if dpkg -l | grep -q "^ii  redis-server "; then
    print_success "Redis 已安装"
else
    print_info "正在安装 Redis..."
    apt-get install -y -qq redis-server
    print_success "Redis 安装完成"
fi

if service redis-server status &>/dev/null; then
    print_success "Redis 服务正在运行"
else
    print_info "正在启动 Redis 服务..."
    service redis-server start
    print_success "Redis 服务已启动"
fi

# 7. 安装 Lua
print_header "7. 安装 Lua"
LUA_PACKAGES=("lua5.3" "liblua5.3-dev" "luarocks")

for pkg in "${LUA_PACKAGES[@]}"; do
    if dpkg -l | grep -q "^ii  $pkg "; then
        print_success "$pkg 已安装"
    else
        print_info "正在安装 $pkg..."
        apt-get install -y -qq "$pkg"
        print_success "$pkg 安装完成"
    fi
done

# 8. 安装 vcpkg
print_header "8. 安装 vcpkg"
VCPKG_DIR="$REAL_HOME/vcpkg"

if [ -d "$VCPKG_DIR" ] && [ -f "$VCPKG_DIR/vcpkg" ]; then
    print_success "vcpkg 已安装在 $VCPKG_DIR"
else
    print_info "正在安装 vcpkg..."
    
    sudo -u $REAL_USER bash << EOF
cd $REAL_HOME
if [ -d "vcpkg" ]; then
    cd vcpkg
    git pull
else
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg
fi
./bootstrap-vcpkg.sh
EOF
    
    print_success "vcpkg 安装完成"
    
    if ! grep -q "VCPKG_ROOT" "$REAL_HOME/.bashrc"; then
        echo "" >> "$REAL_HOME/.bashrc"
        echo "# vcpkg 环境变量" >> "$REAL_HOME/.bashrc"
        echo "export VCPKG_ROOT=\"\$HOME/vcpkg\"" >> "$REAL_HOME/.bashrc"
        echo "export PATH=\"\$VCPKG_ROOT:\$PATH\"" >> "$REAL_HOME/.bashrc"
        print_success "已添加 vcpkg 环境变量到 ~/.bashrc"
    fi
fi

export VCPKG_ROOT="$VCPKG_DIR"
export PATH="$VCPKG_ROOT:$PATH"

# 9. 编译 Skynet
print_header "9. 编译 Skynet"
SKYNET_DIR="$PROJECT_ROOT/skynet_src/skynet"

if [ -d "$SKYNET_DIR" ]; then
    print_success "找到 Skynet 目录: $SKYNET_DIR"
    
    if [ -f "$SKYNET_DIR/skynet" ]; then
        print_success "Skynet 已编译"
    else
        print_info "正在编译 Skynet..."
        sudo -u $REAL_USER bash << EOF
cd "$SKYNET_DIR"
make linux
EOF
        if [ $? -eq 0 ]; then
            print_success "Skynet 编译完成"
        else
            print_error "Skynet 编译失败"
        fi
    fi
else
    print_warning "未找到 Skynet 目录"
fi

# 10. 安装 lua-protobuf
print_header "10. 安装 lua-protobuf 到 Skynet"

THIRD_PARTY_DIR="$SKYNET_DIR/3rd"
mkdir -p "$THIRD_PARTY_DIR"

if [ -f "$SKYNET_DIR/luaclib/pb.so" ]; then
    print_success "lua-protobuf 已安装"
else
    print_info "正在安装 lua-protobuf..."
    
    cd "$THIRD_PARTY_DIR"
    
    if [ -d "lua-protobuf" ]; then
        print_info "lua-protobuf 目录已存在，更新..."
        cd lua-protobuf
        sudo -u $REAL_USER git pull || true
    else
        print_info "正在从 GitHub 克隆 lua-protobuf..."
        sudo -u $REAL_USER git clone https://github.com/starwing/lua-protobuf.git
        cd lua-protobuf
    fi
    
    # 编译
    print_info "正在编译 lua-protobuf..."
    sudo -u $REAL_USER make clean || true
    if sudo -u $REAL_USER make; then
        print_success "lua-protobuf 编译完成"
        
        # 安装
        mkdir -p "$SKYNET_DIR/luaclib"
        cp pb.so "$SKYNET_DIR/luaclib/"
        print_success "pb.so 已安装到 Skynet"
    else
        print_error "lua-protobuf 编译失败"
        print_warning "Skynet 将使用纯 Lua 序列化（性能略低）"
    fi
fi

# 11. 安装项目依赖
print_header "11. 安装项目依赖"

if [ -f "$PROJECT_ROOT/vcpkg.json" ]; then
    print_success "找到 vcpkg.json 配置文件"
    
    print_warning "vcpkg 安装第三方库可能需要 10-60 分钟"
    read -p "是否立即安装项目依赖? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_info "开始安装项目依赖..."
        
        cd "$PROJECT_ROOT"
        sudo -u $REAL_USER bash << EOF
export VCPKG_ROOT="$VCPKG_ROOT"
export PATH="\$VCPKG_ROOT:\$PATH"
cd "$PROJECT_ROOT"
$VCPKG_ROOT/vcpkg install
EOF
        
        if [ $? -eq 0 ]; then
            print_success "项目依赖安装完成"
        else
            print_error "项目依赖安装失败"
        fi
    else
        print_info "跳过依赖安装"
        print_info "稍后可运行: ~/vcpkg/vcpkg install"
    fi
else
    print_warning "未找到 vcpkg.json"
fi

# 12. 最终总结
print_header "安装完成总结"

print_success "WSL2 环境配置完成!"
echo ""
echo "下一步操作："
echo ""
echo "1. 重新加载环境变量:"
echo "   source ~/.bashrc"
echo ""
echo "2. 如果还未安装项目依赖:"
echo "   cd $PROJECT_ROOT"
echo "   ~/vcpkg/vcpkg install"
echo ""
echo "3. 编译项目:"
echo "   bash build.sh Release"
echo ""
echo "4. 启动所有服务:"
echo "   bash tools/service/manage.sh start"
echo ""

print_info "提示:"
echo "  - Skynet 已编译并集成 lua-protobuf"
echo "  - Redis 服务已启动"
echo "  - 使用 tools/service/manage.sh 管理所有服务"
echo ""

print_success "环境准备完成！祝你开发顺利！"
