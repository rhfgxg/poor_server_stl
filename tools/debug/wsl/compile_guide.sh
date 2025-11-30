#!/bin/bash
# WSL2 编译完整指南脚本
# 此脚本会引导您完成整个编译流程

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 打印函数
print_header() {
    echo -e "\n${BLUE}=====================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}=====================================================${NC}\n"
}

print_step() {
    echo -e "${CYAN}[步骤 $1]${NC} $2"
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

# 获取项目目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$(dirname "$SCRIPT_DIR")")")"

print_header "poor_server_stl WSL2 编译指南"
echo "项目目录: $PROJECT_DIR"
echo "当前用户: $USER"
echo ""

# 步骤 1: 检查环境
print_step "1" "检查编译环境"
echo ""

MISSING_TOOLS=()

# 检查 GCC/G++
if command -v gcc &> /dev/null && command -v g++ &> /dev/null; then
    GCC_VERSION=$(gcc --version | head -n 1 | awk '{print $NF}')
    print_success "GCC/G++ 已安装 (版本: $GCC_VERSION)"
else
    print_error "GCC/G++ 未安装"
    MISSING_TOOLS+=("build-essential")
fi

# 检查 CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n 1 | awk '{print $3}')
    print_success "CMake 已安装 (版本: $CMAKE_VERSION)"
else
    print_error "CMake 未安装"
    MISSING_TOOLS+=("cmake")
fi

# 检查 Ninja
if command -v ninja &> /dev/null; then
    NINJA_VERSION=$(ninja --version)
    print_success "Ninja 已安装 (版本: $NINJA_VERSION)"
else
    print_error "Ninja 未安装"
    MISSING_TOOLS+=("ninja-build")
fi

# 检查 protoc
if command -v protoc &> /dev/null; then
    PROTOC_VERSION=$(protoc --version | awk '{print $2}')
    print_success "protoc 已安装 (版本: $PROTOC_VERSION)"
else
    print_error "protoc 未安装"
    MISSING_TOOLS+=("protobuf-compiler")
fi

# 检查 vcpkg
if [ -d "$HOME/vcpkg" ] && [ -f "$HOME/vcpkg/vcpkg" ]; then
    print_success "vcpkg 已安装"
    export VCPKG_ROOT="$HOME/vcpkg"
else
    print_error "vcpkg 未安装"
fi

echo ""

# 如果有缺失的工具，询问是否安装
if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then
    print_warning "检测到缺失的工具："
    for tool in "${MISSING_TOOLS[@]}"; do
        echo "  - $tool"
    done
    echo ""
    read -p "是否运行环境安装脚本？(y/n): " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        if [ -f "$PROJECT_DIR/tools/wsl_environment/setup_wsl2_environment.sh" ]; then
            sudo bash "$PROJECT_DIR/tools/wsl_environment/setup_wsl2_environment.sh"
        else
            print_error "未找到安装脚本！"
            exit 1
        fi
    else
        print_error "缺少必要工具，无法继续编译"
        exit 1
    fi
fi

# 步骤 2: 检查并安装 vcpkg 依赖
print_step "2" "检查 vcpkg 第三方库"
echo ""

cd "$PROJECT_DIR"

if [ ! -d "vcpkg_installed" ]; then
    print_warning "vcpkg 依赖未安装"
    echo ""
    print_info "项目依赖的第三方库："
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
    print_warning "注意：此过程可能需要 30-60 分钟"
    read -p "是否立即安装？(y/n): " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_info "开始安装 vcpkg 依赖..."
        ~/vcpkg/vcpkg install
        if [ $? -eq 0 ]; then
            print_success "vcpkg 依赖安装完成"
        else
            print_error "vcpkg 依赖安装失败"
            exit 1
        fi
    else
        print_error "缺少第三方库，无法编译"
        exit 1
    fi
else
    print_success "vcpkg 依赖已安装"
fi

echo ""

# 步骤 3: 生成 Protobuf 代码
print_step "3" "生成 Protobuf 代码"
echo ""

# 检查 proto 脚本是否存在
if [ -f "$PROJECT_DIR/tools/debug/wsl/proto_make_cpp.sh" ]; then
    print_info "生成 C++ Protobuf 代码..."
    bash "$PROJECT_DIR/tools/debug/wsl/proto_make_cpp.sh"
    if [ $? -eq 0 ]; then
        print_success "C++ Protobuf 代码生成完成"
    else
        print_warning "C++ Protobuf 代码生成失败（可能已存在）"
    fi
else
    print_warning "未找到 proto_make_cpp.sh"
fi

echo ""

if [ -f "$PROJECT_DIR/tools/debug/wsl/proto_make_lua.sh" ]; then
    print_info "生成 Lua Protobuf Descriptor..."
    bash "$PROJECT_DIR/tools/debug/wsl/proto_make_lua.sh"
    if [ $? -eq 0 ]; then
        print_success "Lua Protobuf Descriptor 生成完成"
    else
        print_warning "Lua Protobuf Descriptor 生成失败（可能已存在）"
    fi
else
    print_warning "未找到 proto_make_lua.sh"
fi

echo ""

# 步骤 4: 编译 Skynet（可选）
print_step "4" "编译 Skynet 框架（可选）"
echo ""

SKYNET_DIR="$PROJECT_DIR/skynet_src/skynet"
if [ -d "$SKYNET_DIR" ]; then
    if [ -f "$SKYNET_DIR/skynet" ]; then
        print_success "Skynet 已编译"
    else
        read -p "是否编译 Skynet？(y/n): " -n 1 -r
        echo ""
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            print_info "正在编译 Skynet..."
            cd "$SKYNET_DIR"
            make linux
            if [ $? -eq 0 ]; then
                print_success "Skynet 编译完成"
            else
                print_error "Skynet 编译失败"
            fi
            cd "$PROJECT_DIR"
        fi
    fi
else
    print_info "未找到 Skynet 目录，跳过"
fi

echo ""

# 步骤 5: 选择编译模式
print_step "5" "选择编译模式"
echo ""
echo "1) Debug   - 调试模式（包含调试信息）"
echo "2) Release - 发布模式（优化性能）✨ 推荐"
echo ""
print_warning "注意：WSL 环境下推荐使用 Release 模式"
print_info "原因：Debug 模式可能因 MySQL Connector 的 protobuf 冲突导致链接失败"
print_info "详情参考：docunment/项目配置与运行/WSL_编译已知问题.md"
echo ""
read -p "请选择编译模式 (1/2，默认为2): " -n 1 -r
echo ""

BUILD_TYPE="Release"
if [[ $REPLY == "1" ]]; then
    BUILD_TYPE="Debug"
    print_warning "已选择 Debug 模式"
    print_info "如遇到 protobuf 符号冲突错误，请使用 Release 模式"
else
    BUILD_TYPE="Release"
    print_success "已选择 Release 模式（推荐）"
fi

echo ""

# 步骤 6: 编译项目
print_step "6" "编译项目"
echo ""

BUILD_DIR="$PROJECT_DIR/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

print_info "配置 CMake..."
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
    "$PROJECT_DIR"

if [ $? -ne 0 ]; then
    print_error "CMake 配置失败"
    exit 1
fi

echo ""
print_info "开始编译（这可能需要几分钟）..."
echo ""

ninja

if [ $? -eq 0 ]; then
    echo ""
    print_success "编译成功！"
else
    echo ""
    print_error "编译失败！请检查错误信息"
    exit 1
fi

# 步骤 7: 显示编译结果
print_header "编译完成！"

echo "可执行文件位置："
echo ""

EXECUTABLES=(
    "src/gateway/gateway:网关服务器"
    "src/login/login:登录服务器"
    "src/db/db:数据库服务器"
    "src/file/file:文件服务器"
    "src/central/central:中央服务器"
    "src/matching/matching:匹配服务器"
)

for exe in "${EXECUTABLES[@]}"; do
    IFS=':' read -r path desc <<< "$exe"
    if [ -f "$BUILD_DIR/$path" ]; then
        SIZE=$(du -h "$BUILD_DIR/$path" | cut -f1)
        print_success "$desc: ./$path ($SIZE)"
    else
        print_warning "$desc: 未生成"
    fi
done

# 步骤 8: 后续步骤提示
echo ""
print_header "下一步操作"

echo "1. 启动数据库服务："
echo "   ${CYAN}bash $PROJECT_DIR/start_services.sh${NC}"
echo ""

echo "2. 配置 MySQL（如果使用 Windows MySQL）："
echo "   ${CYAN}mysql -h 127.0.0.1 -u root -p${NC}"
echo ""

echo "3. 创建数据库和数据表："
echo "   参考: docunment/server/数据库/sql定义文件"
echo ""

echo "4. 复制配置文件（如果需要）："
echo "   ${CYAN}cp -r $PROJECT_DIR/config $BUILD_DIR/${NC}"
echo ""

echo "5. 运行服务器："
echo "   ${CYAN}cd $BUILD_DIR${NC}"
echo "   ${CYAN}./src/gateway/gateway &${NC}"
echo "   ${CYAN}./src/login/login &${NC}"
echo "   ${CYAN}./src/db/db &${NC}"
echo ""

print_success "编译指南执行完毕！"
echo ""
