#!/bin/bash
# 项目编译脚本 for WSL2/Linux

set -e

# 获取项目根目录（脚本在 tools/debug/wsl/ 目录下）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$(dirname "$SCRIPT_DIR")")")"

# 获取构建类型（默认 Debug）
BUILD_TYPE=${1:-Debug}

echo "========================================"
echo "  poor_server_stl 编译脚本 (WSL/Linux)"
echo "========================================"
echo ""
echo "项目目录: $PROJECT_DIR"
echo "构建类型: $BUILD_TYPE"
echo ""

# 检查 vcpkg
if [ ! -d "$HOME/vcpkg" ]; then
    echo "错误: 未找到 vcpkg！"
    echo "请运行: sudo bash tools/wsl_environment/setup_wsl2_environment.sh"
    exit 1
fi

# 检查 vcpkg 是否已安装依赖
if [ ! -d "$PROJECT_DIR/vcpkg_installed" ]; then
    echo "警告: vcpkg 依赖未安装！"
    echo "正在安装项目依赖..."
    cd "$PROJECT_DIR"
    ~/vcpkg/vcpkg install
fi

# 创建并进入构建目录
BUILD_DIR="$PROJECT_DIR/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "开始配置 CMake..."
echo ""

# 配置 CMake
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
    "$PROJECT_DIR"

echo ""
echo "开始编译..."
echo ""

# 编译
ninja

echo ""
echo "========================================"
echo "  ✓ 编译完成!"
echo "========================================"
echo ""
echo "可执行文件位置:"
echo "  Gateway:  $BUILD_DIR/src/gateway/gateway"
echo "  Login:    $BUILD_DIR/src/login/login"
echo "  DB:       $BUILD_DIR/src/db/db"
echo "  File:     $BUILD_DIR/src/file/file"
echo "  Central:  $BUILD_DIR/src/central/central"
echo "  Matching: $BUILD_DIR/src/matching/matching"
echo ""
echo "下一步:"
echo "  1. 启动数据库服务: bash $PROJECT_DIR/start_services.sh"
echo "  2. 运行服务器: ./build/src/gateway/gateway"
echo ""
