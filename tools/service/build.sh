#!/bin/bash
# 项目编译脚本
# 编译 Skynet 和 C++ 项目

set -e

# 加载公共函数
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

BUILD_TYPE="${1:-Release}"

#==================== 编译 Skynet ====================#

compile_skynet() {
    print_header "编译 Skynet"
    
    if ! dir_exists "$SKYNET_DIR"; then
        print_error "Skynet 目录不存在: $SKYNET_DIR"
        return 1
    fi
    
    if file_exists "$SKYNET_DIR/skynet"; then
        print_success "Skynet 已编译"
        return 0
    fi
    
    print_info "正在编译 Skynet..."
    cd "$SKYNET_DIR"
    make linux
    
    if file_exists "$SKYNET_DIR/skynet"; then
        print_success "Skynet 编译完成"
    else
        print_error "Skynet 编译失败"
        return 1
    fi
}

#==================== 编译 C++ ====================#

compile_cpp() {
    print_header "编译 C++ 项目 ($BUILD_TYPE)"
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    print_info "配置 CMake..."
    cmake -G Ninja \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" \
        "$PROJECT_ROOT"
    
    if [ $? -ne 0 ]; then
        print_error "CMake 配置失败"
        return 1
    fi
    
    print_info "开始编译..."
    ninja
    
    if [ $? -eq 0 ]; then
        print_success "C++ 项目编译完成"
    else
        print_error "C++ 项目编译失败"
        return 1
    fi
}

#==================== 主流程 ====================#

print_header "项目编译 ($BUILD_TYPE)"

# 1. 编译 Skynet
compile_skynet || exit 1

# 2. 编译 C++
compile_cpp || exit 1

print_header "编译完成"
print_success "所有组件编译成功！"
