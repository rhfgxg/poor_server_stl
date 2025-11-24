#!/bin/bash
# WSL2 (Ubuntu) 环境检查和安装脚本
# 用于自动检查和安装 poor_server_stl 项目所需的所有依赖

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印函数
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

# 检查是否以 root 权限运行
check_root() {
    if [ "$EUID" -ne 0 ]; then 
        print_error "请使用 sudo 运行此脚本"
        echo "使用方法: sudo bash setup_wsl2_environment.sh"
        exit 1
    fi
}

# 获取实际用户（即使使用 sudo 运行）
REAL_USER=${SUDO_USER:-$USER}
REAL_HOME=$(eval echo ~$REAL_USER)

print_header "WSL2 (Ubuntu) 环境检查和安装脚本"
echo "项目: poor_server_stl"
echo "目标平台: Ubuntu (WSL2)"
echo "当前用户: $REAL_USER"
echo "用户主目录: $REAL_HOME"

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
    "build-essential"  # GCC/G++ 编译器
    "cmake"            # CMake 构建工具
    "ninja-build"      # Ninja 构建系统
    "git"              # Git 版本控制
    "curl"             # 下载工具
    "wget"             # 下载工具
    "pkg-config"       # 包配置工具
    "autoconf"         # 自动配置工具
    "automake"         # 自动化构建工具
    "autoconf-archive" # autoconf 宏集合
    "libtool"          # 库工具
    "m4"               # 宏处理器
    "make"             # Make 构建工具
    "unzip"            # 解压工具
    "zip"              # 压缩工具
    "tar"              # 打包工具
    "clang"            # Clang 编译器（可选）
    "tree"             # 文件树查看工具
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
print_header "4. 安装 Protocol Buffers 编译器"
if command -v protoc &> /dev/null; then
    PROTOC_VERSION=$(protoc --version | awk '{print $2}')
    print_success "protoc 已安装 (版本: $PROTOC_VERSION)"
else
    print_info "正在安装 protobuf-compiler..."
    apt-get install -y -qq protobuf-compiler libprotobuf-dev
    print_success "protobuf-compiler 安装完成"
fi

# 5. 安装 gRPC
print_header "5. 检查 gRPC 工具"
if command -v grpc_cpp_plugin &> /dev/null; then
    print_success "grpc_cpp_plugin 已安装"
else
    print_warning "grpc_cpp_plugin 未找到（将通过 vcpkg 安装）"
fi

# 6. 安装 MySQL 客户端
print_header "6. 安装 MySQL 客户端和开发库"
if dpkg -l | grep -q "^ii  mysql-client "; then
    print_success "MySQL 客户端已安装"
else
    print_info "正在安装 MySQL 客户端..."
    apt-get install -y -qq mysql-client libmysqlclient-dev
    print_success "MySQL 客户端安装完成"
fi

# 检查 MySQL 服务器
print_info "检查 MySQL 服务器..."
if systemctl is-active --quiet mysql 2>/dev/null || service mysql status &>/dev/null; then
    print_success "MySQL 服务器正在运行"
else
    print_warning "MySQL 服务器未运行"
    read -p "是否安装并启动 MySQL 服务器? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_info "正在安装 MySQL 服务器..."
        apt-get install -y -qq mysql-server
        service mysql start
        print_success "MySQL 服务器已安装并启动"
        print_warning "请运行 'mysql_secure_installation' 来配置 MySQL"
    else
        print_info "跳过 MySQL 服务器安装（可使用 Windows 的 MySQL 服务）"
        print_info "连接方式: mysql -h 127.0.0.1 -u <用户名> -p"
    fi
fi

# 7. 安装 Redis
print_header "7. 安装 Redis"
if dpkg -l | grep -q "^ii  redis-server "; then
    print_success "Redis 已安装"
else
    print_info "正在安装 Redis..."
    apt-get install -y -qq redis-server
    print_success "Redis 安装完成"
fi

# 检查 Redis 服务
if service redis-server status &>/dev/null; then
    print_success "Redis 服务正在运行"
else
    print_info "正在启动 Redis 服务..."
    service redis-server start
    print_success "Redis 服务已启动"
fi

# 8. 安装 Lua
print_header "8. 安装 Lua 和开发库"
LUA_PACKAGES=(
    "lua5.3"
    "liblua5.3-dev"
    "luarocks"
)

for pkg in "${LUA_PACKAGES[@]}"; do
    if dpkg -l | grep -q "^ii  $pkg "; then
        print_success "$pkg 已安装"
    else
        print_info "正在安装 $pkg..."
        apt-get install -y -qq "$pkg"
        print_success "$pkg 安装完成"
    fi
done

# 9. 安装/检查 vcpkg
print_header "9. 检查 vcpkg 包管理器"
VCPKG_DIR="$REAL_HOME/vcpkg"

if [ -d "$VCPKG_DIR" ] && [ -f "$VCPKG_DIR/vcpkg" ]; then
    print_success "vcpkg 已安装在 $VCPKG_DIR"
else
    print_info "vcpkg 未找到，正在安装..."
    
    # 切换到实际用户执行
    sudo -u $REAL_USER bash << EOF
cd $REAL_HOME
if [ -d "vcpkg" ]; then
    print_warning "vcpkg 目录已存在但未完成安装，正在重新初始化..."
    cd vcpkg
    git pull
else
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg
fi
./bootstrap-vcpkg.sh
EOF
    
    print_success "vcpkg 安装完成"
    
    # 添加到用户的 bashrc
    if ! grep -q "VCPKG_ROOT" "$REAL_HOME/.bashrc"; then
        echo "" >> "$REAL_HOME/.bashrc"
        echo "# vcpkg 环境变量" >> "$REAL_HOME/.bashrc"
        echo "export VCPKG_ROOT=\"\$HOME/vcpkg\"" >> "$REAL_HOME/.bashrc"
        echo "export PATH=\"\$VCPKG_ROOT:\$PATH\"" >> "$REAL_HOME/.bashrc"
        print_success "已添加 vcpkg 环境变量到 ~/.bashrc"
    fi
fi

# 设置临时环境变量
export VCPKG_ROOT="$VCPKG_DIR"
export PATH="$VCPKG_ROOT:$PATH"

# 10. 安装 vcpkg 依赖的工具
print_header "10. 安装 vcpkg 所需工具"
print_info "确保 vcpkg 所需的所有工具已安装..."

# 先安装 Perl（非常重要！）
print_info "安装 Perl 解释器..."
apt-get install -y perl perl-base perl-modules-5.38

# 验证 Perl 安装
if ! command -v perl &> /dev/null; then
    print_error "Perl 命令不可用！尝试修复..."
    
    # 方法 1: 重新配置包
    dpkg --configure -a
    apt-get install -f -y
    
    # 方法 2: 重新安装
    apt-get install --reinstall -y perl-base perl
    
    # 方法 3: 如果还是失败，查找并创建链接
    if ! command -v perl &> /dev/null; then
        PERL_PATH=$(find /usr -type f -name "perl" -executable 2>/dev/null | head -n 1)
        if [ -n "$PERL_PATH" ]; then
            print_warning "找到 Perl 在 $PERL_PATH，创建符号链接..."
            ln -sf "$PERL_PATH" /usr/bin/perl
            chmod +x /usr/bin/perl
        else
            print_error "系统中未找到 Perl 可执行文件"
            print_warning "将尝试使用系统的 protobuf 而不是 vcpkg 编译"
            apt-get install -y protobuf-compiler libprotobuf-dev
        fi
    fi
fi

# 最终验证 Perl
if command -v perl &> /dev/null; then
    PERL_VERSION=$(perl --version 2>/dev/null | grep -oP 'v\d+\.\d+\.\d+' | head -n 1 || echo "未知")
    print_success "Perl 已安装 ($PERL_VERSION)"
else
    print_error "Perl 安装失败！vcpkg 可能无法编译某些包"
    print_warning "建议：使用系统的 protobuf 或手动修复 Perl"
fi

# 安装其他工具
print_info "安装 autotools 和相关工具..."
apt-get install -y \
    pkg-config \
    autoconf \
    automake \
    autoconf-archive \
    libtool \
    m4 \
    make \
    gettext \
    autopoint \
    autotools-dev \
    libtool-bin

print_info "验证关键工具..."
if command -v autoreconf &> /dev/null; then
    if autoreconf --version &>/dev/null; then
        AUTORECONF_VERSION=$(autoreconf --version 2>/dev/null | head -n 1 | grep -oP '\d+\.\d+' || echo "未知")
        print_success "autoreconf 可用 (版本: $AUTORECONF_VERSION)"
    else
        print_warning "autoreconf 存在但无法执行"
    fi
else
    print_warning "autoreconf 不可用，尝试修复..."
    apt-get install --reinstall -y autoconf automake libtool
    
    # 再次验证
    if command -v autoreconf &> /dev/null && autoreconf --version &>/dev/null; then
        print_success "autoreconf 修复成功"
    else
        print_error "autoreconf 仍然不可用"
        print_info "可能需要手动修复，请查看文档: docunment/项目配置与运行/WSL2_vcpkg修复指南.md"
    fi
fi

# 11. 检查项目目录
print_header "11. 检查项目配置"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"

print_info "脚本目录: $SCRIPT_DIR"
print_info "项目目录: $PROJECT_DIR"

if [ -f "$PROJECT_DIR/vcpkg.json" ]; then
    print_success "找到 vcpkg.json 配置文件"
    echo ""
    print_info "项目依赖的第三方库:"
    cat "$PROJECT_DIR/vcpkg.json" | grep -A 100 "dependencies" | grep '"' | sed 's/[",]//g' | sed 's/^/    - /'
else
    print_warning "未找到 vcpkg.json 配置文件"
fi

# 12. 询问是否安装项目依赖
print_header "12. 安装项目第三方库"
echo "项目依赖的第三方库包括："
echo "  - abseil"
echo "  - nlohmann-json"
echo "  - mysql-connector-cpp"
echo "  - boost"
echo "  - grpc"
echo "  - cpp-redis"
echo "  - tacopie"
echo "  - spdlog"
echo "  - lua"
echo "  - jwt-cpp"
echo ""
print_warning "注意: vcpkg 安装第三方库可能需要较长时间（10-60分钟）"

read -p "是否立即使用 vcpkg 安装项目依赖? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    print_info "开始安装项目依赖..."
    
    cd "$PROJECT_DIR"
    sudo -u $REAL_USER bash << EOF
export VCPKG_ROOT="$VCPKG_ROOT"
export PATH="\$VCPKG_ROOT:\$PATH"
cd "$PROJECT_DIR"
$VCPKG_ROOT/vcpkg install
EOF
    
    if [ $? -eq 0 ]; then
        print_success "项目依赖安装完成"
    else
        print_error "项目依赖安装失败，请检查错误信息"
    fi
else
    print_info "跳过依赖安装"
    print_info "稍后可在项目目录下运行: $VCPKG_ROOT/vcpkg install"
fi

# 13. 编译 Skynet（如果存在）
print_header "13. 检查 Skynet 框架"
SKYNET_DIR="$PROJECT_DIR/skynet_src/skynet"

if [ -d "$SKYNET_DIR" ]; then
    print_success "找到 Skynet 目录: $SKYNET_DIR"
    
    if [ -f "$SKYNET_DIR/skynet" ]; then
        print_success "Skynet 已编译"
    else
        read -p "是否编译 Skynet? (y/n): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
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
    fi
else
    print_warning "未找到 Skynet 目录"
fi

# 14. 生成环境报告
print_header "14. 环境检查报告"

echo ""
echo "=== 编译工具 ==="
gcc --version | head -n 1
g++ --version | head -n 1
cmake --version | head -n 1
echo "Ninja: $(ninja --version)"

echo ""
echo "=== Protocol Buffers ==="
protoc --version

echo ""
echo "=== 数据库 ==="
if command -v mysql &> /dev/null; then
    mysql --version
else
    echo "MySQL 客户端: 未安装"
fi

if command -v redis-cli &> /dev/null; then
    redis-cli --version
else
    echo "Redis 客户端: 未安装"
fi

echo ""
echo "=== Lua ==="
if command -v lua &> /dev/null; then
    lua -v
else
    echo "Lua: 未安装"
fi

echo ""
echo "=== vcpkg ==="
if [ -f "$VCPKG_ROOT/vcpkg" ]; then
    $VCPKG_ROOT/vcpkg version
else
    echo "vcpkg: 未安装"
fi

# 15. 创建项目便捷脚本
print_header "15. 创建项目便捷脚本"

# 创建启动服务脚本
cat > "$PROJECT_DIR/start_services.sh" << 'EOFSTART'
#!/bin/bash
# 启动数据库服务脚本

echo "启动 Redis 服务..."
sudo service redis-server start

echo "检查服务状态..."
sudo service redis-server status

echo "Redis 服务已启动!"
echo ""
echo "注意: 如果使用 Windows 的 MySQL，无需在 WSL2 中启动"
echo "连接方式: mysql -h 127.0.0.1 -u <用户名> -p"
EOFSTART

chmod +x "$PROJECT_DIR/start_services.sh"
print_success "已创建服务启动脚本: $PROJECT_DIR/start_services.sh"

# 16. 最终总结
print_header "安装完成总结"

print_success "WSL2 环境配置完成!"
echo ""
echo "下一步操作："
echo ""
echo "1. 重新加载环境变量:"
echo "   source ~/.bashrc"
echo ""
echo "2. 如果还未安装项目依赖，运行:"
echo "   cd $PROJECT_DIR"
echo "   ~/vcpkg/vcpkg install"
echo ""
echo "3. 编译 Proto 文件:"
echo "   cd $PROJECT_DIR"
echo "   bash tools/debug/wsl/proto_make_cpp.sh"
echo "   bash tools/debug/wsl/proto_make_lua.sh"
echo ""
echo "4. 编译项目（推荐使用交互式编译指南）:"
echo "   cd $PROJECT_DIR"
echo "   bash tools/debug/wsl/compile_guide.sh"
echo ""
echo "   或快速编译:"
echo "   bash tools/debug/wsl/build.sh Release"
echo ""
echo "5. 启动数据库服务:"
echo "   bash start_services.sh"
echo ""

print_info "提示: 如需使用 Windows 的 MySQL，连接方式:"
echo "   mysql -h 127.0.0.1 -u <用户名> -p"
echo ""

print_success "环境准备完成！祝你开发顺利！"
