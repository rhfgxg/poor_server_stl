#!/bin/bash
# WSL2 环境卸载脚本
# 用于清理 setup_wsl2_environment.sh 安装的内容

set -e

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

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[i]${NC} $1"
}

print_header "WSL2 环境清理脚本"

echo -e "${RED}警告: 此脚本将删除开发环境相关的软件包和配置${NC}"
echo ""
echo "将会清理："
echo "  - 开发工具 (gcc, cmake, ninja, etc.)"
echo "  - Protocol Buffers"
echo "  - MySQL 和 Redis"
echo "  - Lua"
echo "  - vcpkg"
echo "  - 项目依赖库"
echo ""

read -p "确定要继续吗？(输入 YES 继续): " -r
if [ "$REPLY" != "YES" ]; then
    echo "已取消"
    exit 0
fi

# 检查 root 权限
if [ "$EUID" -ne 0 ]; then 
    print_warning "需要 sudo 权限，请使用: sudo bash uninstall_wsl2_environment.sh"
    exit 1
fi

REAL_USER=${SUDO_USER:-$USER}
REAL_HOME=$(eval echo ~$REAL_USER)

# 1. 删除项目生成的脚本
print_header "1. 清理项目脚本"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"

if [ -f "$PROJECT_DIR/build.sh" ]; then
    rm -f "$PROJECT_DIR/build.sh"
    print_info "已删除 build.sh"
fi

if [ -f "$PROJECT_DIR/start_services.sh" ]; then
    rm -f "$PROJECT_DIR/start_services.sh"
    print_info "已删除 start_services.sh"
fi

# 2. 询问是否删除 vcpkg
print_header "2. vcpkg 包管理器"
read -p "是否删除 vcpkg? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ -d "$REAL_HOME/vcpkg" ]; then
        print_info "正在删除 vcpkg..."
        rm -rf "$REAL_HOME/vcpkg"
        print_info "vcpkg 已删除"
        
        # 从 bashrc 删除环境变量
        if [ -f "$REAL_HOME/.bashrc" ]; then
            sed -i '/# vcpkg 环境变量/d' "$REAL_HOME/.bashrc"
            sed -i '/VCPKG_ROOT/d' "$REAL_HOME/.bashrc"
            print_info "已从 ~/.bashrc 移除 vcpkg 环境变量"
        fi
    else
        print_info "vcpkg 未安装"
    fi
fi

# 3. 询问是否删除项目依赖
print_header "3. 项目依赖库"
if [ -d "$PROJECT_DIR/vcpkg_installed" ]; then
    read -p "是否删除项目依赖库 (vcpkg_installed)? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_info "正在删除 vcpkg_installed..."
        rm -rf "$PROJECT_DIR/vcpkg_installed"
        print_info "项目依赖库已删除"
    fi
fi

# 4. 询问是否卸载 MySQL
print_header "4. MySQL"
read -p "是否卸载 MySQL? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    print_info "正在卸载 MySQL..."
    systemctl stop mysql 2>/dev/null || service mysql stop 2>/dev/null || true
    apt-get purge -y mysql-server mysql-client mysql-common libmysqlclient-dev
    apt-get autoremove -y
    rm -rf /var/lib/mysql
    rm -rf /etc/mysql
    print_info "MySQL 已卸载"
fi

# 5. 询问是否卸载 Redis
print_header "5. Redis"
read -p "是否卸载 Redis? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    print_info "正在卸载 Redis..."
    systemctl stop redis-server 2>/dev/null || service redis-server stop 2>/dev/null || true
    apt-get purge -y redis-server
    apt-get autoremove -y
    print_info "Redis 已卸载"
fi

# 6. 询问是否卸载 Lua
print_header "6. Lua"
read -p "是否卸载 Lua? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    print_info "正在卸载 Lua..."
    apt-get purge -y lua5.3 liblua5.3-dev luarocks
    apt-get autoremove -y
    print_info "Lua 已卸载"
fi

# 7. 询问是否卸载 Protocol Buffers
print_header "7. Protocol Buffers"
read -p "是否卸载 Protocol Buffers? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    print_info "正在卸载 Protocol Buffers..."
    apt-get purge -y protobuf-compiler libprotobuf-dev
    apt-get autoremove -y
    print_info "Protocol Buffers 已卸载"
fi

# 8. 询问是否卸载基础开发工具
print_header "8. 基础开发工具"
echo "基础工具包括: gcc, g++, cmake, ninja, git, clang 等"
read -p "是否卸载基础开发工具? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    print_info "正在卸载基础开发工具..."
    apt-get purge -y \
        build-essential \
        cmake \
        ninja-build \
        clang \
        tree \
        pkg-config \
        autoconf \
        automake \
        autoconf-archive \
        libtool \
        m4
    apt-get autoremove -y
    print_info "基础开发工具已卸载"
fi

# 9. 清理 APT 缓存
print_header "9. 清理 APT 缓存"
read -p "是否清理 APT 缓存? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    apt-get clean
    apt-get autoclean
    print_info "APT 缓存已清理"
fi

# 10. 清理编译产物
print_header "10. 清理编译产物"
if [ -d "$PROJECT_DIR/build" ]; then
    read -p "是否删除项目编译产物 (build/)? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf "$PROJECT_DIR/build"
        print_info "编译产物已删除"
    fi
fi

if [ -d "$PROJECT_DIR/out" ]; then
    read -p "是否删除项目输出目录 (out/)? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf "$PROJECT_DIR/out"
        print_info "输出目录已删除"
    fi
fi

# 完成
print_header "清理完成"
echo ""
echo -e "${GREEN}环境清理已完成！${NC}"
echo ""
echo "如需重新安装，请运行:"
echo "  sudo bash tools/debug/setup_wsl2_environment.sh"
echo ""
