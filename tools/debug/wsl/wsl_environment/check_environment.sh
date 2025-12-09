#!/bin/bash
# WSL2 环境快速检查脚本
# 检查项目所需的所有依赖是否已正确安装

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_check() {
    local name=$1
    local command=$2
    
    echo -n "检查 $name ... "
    if eval "$command" &>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        return 0
    else
        echo -e "${RED}✗${NC}"
        return 1
    fi
}

print_version() {
    local name=$1
    local command=$2
    
    echo -n "$name: "
    if result=$(eval "$command" 2>/dev/null); then
        echo -e "${GREEN}$result${NC}"
        return 0
    else
        echo -e "${RED}未安装${NC}"
        return 1
    fi
}

echo -e "${BLUE}================================${NC}"
echo -e "${BLUE}WSL2 环境检查${NC}"
echo -e "${BLUE}================================${NC}"
echo ""

# 系统信息
echo "=== 系统信息 ==="
echo "操作系统: $(lsb_release -d | cut -f2)"
echo "内核: $(uname -r)"
echo "架构: $(uname -m)"
echo ""

# 基础工具
echo "=== 基础开发工具 ==="
print_check "GCC" "gcc --version"
print_check "G++" "g++ --version"
print_check "CMake" "cmake --version"
print_check "Ninja" "ninja --version"
print_check "Git" "git --version"
print_check "Make" "make --version"
print_check "pkg-config" "pkg-config --version"
echo ""

# 编译器版本
echo "=== 编译器版本 ==="
print_version "GCC" "gcc --version | head -n 1"
print_version "G++" "g++ --version | head -n 1"
print_version "CMake" "cmake --version | head -n 1"
echo ""

# Protobuf 和 gRPC
echo "=== Protocol Buffers & gRPC ==="
print_check "protoc" "command -v protoc"
print_version "protoc" "protoc --version"
print_check "grpc_cpp_plugin" "command -v grpc_cpp_plugin"
echo ""

# 数据库
echo "=== 数据库 ==="
print_check "MySQL 客户端" "command -v mysql"
print_check "Redis 客户端" "command -v redis-cli"
echo ""

if command -v mysql &>/dev/null; then
    echo -n "MySQL 服务: "
    if sudo service mysql status &>/dev/null; then
        echo -e "${GREEN}运行中${NC}"
    else
        echo -e "${YELLOW}未运行${NC}"
    fi
fi

if command -v redis-cli &>/dev/null; then
    echo -n "Redis 服务: "
    if sudo service redis-server status &>/dev/null; then
        echo -e "${GREEN}运行中${NC}"
    else
        echo -e "${YELLOW}未运行${NC}"
    fi
fi
echo ""

# Lua
echo "=== Lua 环境 ==="
print_check "Lua" "command -v lua"
print_version "Lua" "lua -v 2>&1"
print_check "LuaRocks" "command -v luarocks"
echo ""

# vcpkg
echo "=== vcpkg 包管理器 ==="
VCPKG_ROOT=${VCPKG_ROOT:-$HOME/vcpkg}
echo "VCPKG_ROOT: $VCPKG_ROOT"

if [ -f "$VCPKG_ROOT/vcpkg" ]; then
    echo -e "vcpkg: ${GREEN}已安装${NC}"
    $VCPKG_ROOT/vcpkg version | head -n 1
else
    echo -e "vcpkg: ${RED}未安装${NC}"
fi
echo ""

# 项目配置
echo "=== 项目配置 ==="
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"

echo "项目目录: $PROJECT_DIR"

if [ -f "$PROJECT_DIR/vcpkg.json" ]; then
    echo -e "vcpkg.json: ${GREEN}存在${NC}"
else
    echo -e "vcpkg.json: ${RED}不存在${NC}"
fi

if [ -f "$PROJECT_DIR/CMakeLists.txt" ]; then
    echo -e "CMakeLists.txt: ${GREEN}存在${NC}"
else
    echo -e "CMakeLists.txt: ${RED}不存在${NC}"
fi

# Skynet
SKYNET_DIR="$PROJECT_DIR/skynet_src/skynet"
if [ -d "$SKYNET_DIR" ]; then
    echo -e "Skynet 目录: ${GREEN}存在${NC}"
    if [ -f "$SKYNET_DIR/skynet" ]; then
        echo -e "Skynet 可执行文件: ${GREEN}已编译${NC}"
    else
        echo -e "Skynet 可执行文件: ${YELLOW}未编译${NC}"
    fi
else
    echo -e "Skynet 目录: ${YELLOW}不存在${NC}"
fi
echo ""

# vcpkg 依赖安装状态
echo "=== 项目依赖状态 ==="
if [ -d "$PROJECT_DIR/vcpkg_installed" ]; then
    echo -e "vcpkg_installed: ${GREEN}存在${NC}"
    
    # 检查关键库
    if [ -d "$PROJECT_DIR/vcpkg_installed/x64-linux" ]; then
        echo "已安装的库（x64-linux）:"
        ls "$PROJECT_DIR/vcpkg_installed/x64-linux/lib" 2>/dev/null | grep -E "\.a$|\.so$" | head -n 5
        echo "..."
    fi
else
    echo -e "vcpkg_installed: ${YELLOW}不存在 (需要运行 vcpkg install)${NC}"
fi
echo ""

# 总结
echo "=== 检查总结 ==="
MISSING=0

command -v gcc &>/dev/null || ((MISSING++))
command -v g++ &>/dev/null || ((MISSING++))
command -v cmake &>/dev/null || ((MISSING++))
command -v protoc &>/dev/null || ((MISSING++))
command -v mysql &>/dev/null || ((MISSING++))
command -v redis-cli &>/dev/null || ((MISSING++))
[ -f "$VCPKG_ROOT/vcpkg" ] || ((MISSING++))

if [ $MISSING -eq 0 ]; then
    echo -e "${GREEN}所有必要工具已安装！${NC}"
    echo ""
    echo "建议的下一步操作："
    echo "1. 如果 vcpkg_installed 不存在，运行:"
    echo "   cd $PROJECT_DIR && ~/vcpkg/vcpkg install"
    echo "2. 编译 Proto 文件:"
    echo "   bash tools/debug/proto_make_cpp.sh"
    echo "   bash tools/debug/proto_make_lua.sh"
    echo "3. 编译项目:"
    echo "   bash build.sh"
else
    echo -e "${YELLOW}缺少 $MISSING 个必要工具${NC}"
    echo ""
    echo "请运行以下命令安装所有依赖:"
    echo "  sudo bash tools/debug/setup_wsl2_environment.sh"
fi
