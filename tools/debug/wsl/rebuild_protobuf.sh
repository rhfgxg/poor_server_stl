#!/bin/bash
# WSL2 Protobuf 重新生成和项目重新编译脚本
# 解决 protobuf 版本不匹配问题

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

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"

print_header "WSL2 Protobuf 重新生成脚本"

# 步骤 1：删除旧的 protobuf 生成文件
print_header "步骤 1/6: 删除旧的 Protobuf 生成文件"
print_info "删除 protobuf/cpp/src/*.pb.*"

cd "$PROJECT_DIR" || { print_error "无法进入项目目录"; exit 1; }

rm -f protobuf/cpp/src/*.pb.h
rm -f protobuf/cpp/src/*.pb.cc
rm -f protobuf/cpp/src/*.grpc.pb.h
rm -f protobuf/cpp/src/*.grpc.pb.cc

print_success "已删除所有旧的 protobuf 文件"

# 步骤 2：检查 protoc 和 grpc_cpp_plugin
print_header "步骤 2/6: 检查 Protoc 和 gRPC 插件"

# 设置库路径（在开始时就设置，确保所有工具都能找到依赖）
export LD_LIBRARY_PATH="$PROJECT_DIR/vcpkg_installed/x64-linux/lib:$LD_LIBRARY_PATH"
print_info "设置 LD_LIBRARY_PATH: vcpkg_installed/x64-linux/lib"

# 查找 protoc - 优先使用 vcpkg 的版本
PROTOC=""
if [ -f "vcpkg_installed/x64-linux/tools/protobuf/protoc" ]; then
    PROTOC="$PROJECT_DIR/vcpkg_installed/x64-linux/tools/protobuf/protoc"
    print_success "找到 vcpkg protoc"
elif command -v protoc &>/dev/null; then
    PROTOC="protoc"
    print_warning "使用系统 protoc: $(which protoc)"
else
    print_error "未找到 protoc！"
    exit 1
fi

# 检查版本
PROTOC_VERSION=$("$PROTOC" --version 2>&1 || echo "无法获取版本")
print_info "Protoc 版本: $PROTOC_VERSION"

# 验证 protoc 可执行
print_info "验证 protoc..."
if ! "$PROTOC" --version &>/dev/null; then
    print_error "protoc 无法执行"
    ldd "$PROTOC" 2>&1 | grep "not found" || echo "依赖检查未发现问题"
    exit 1
fi
print_success "protoc 验证通过"

# 查找 grpc_cpp_plugin
GRPC_PLUGIN=""
if [ -f "vcpkg_installed/x64-linux/tools/grpc/grpc_cpp_plugin" ]; then
    GRPC_PLUGIN="$PROJECT_DIR/vcpkg_installed/x64-linux/tools/grpc/grpc_cpp_plugin"
    print_success "找到 gRPC 插件"
    
    # 确保有执行权限
    if [ ! -x "$GRPC_PLUGIN" ]; then
        print_info "添加执行权限..."
        chmod +x "$GRPC_PLUGIN" || { print_error "无法添加执行权限"; exit 1; }
    fi
elif command -v grpc_cpp_plugin &>/dev/null; then
    GRPC_PLUGIN="grpc_cpp_plugin"
    print_success "找到系统 gRPC 插件"
else
    print_error "未找到 grpc_cpp_plugin！"
    exit 1
fi

print_success "grpc_cpp_plugin 验证通过"

# 步骤 3：重新生成所有 proto 文件
print_header "步骤 3/6: 重新生成 Protobuf 文件"

PROTO_DIR="protobuf/cpp"
OUT_DIR="protobuf/cpp/src"

# 确保输出目录存在
mkdir -p "$OUT_DIR" || { print_error "无法创建输出目录"; exit 1; }

# 获取所有 proto 文件
if [ ! -d "$PROTO_DIR" ]; then
    print_error "Proto 目录不存在: $PROTO_DIR"
    exit 1
fi

# 使用更可靠的方式获取 proto 文件列表
shopt -s nullglob
proto_files=("$PROTO_DIR"/*.proto)
shopt -u nullglob

if [ ${#proto_files[@]} -eq 0 ]; then
    print_error "未找到 proto 文件: $PROTO_DIR/*.proto"
    ls -la "$PROTO_DIR/"
    exit 1
fi

total_files=${#proto_files[@]}
print_info "找到 $total_files 个 proto 文件"

current=0
failed=0
failed_files=()

for proto_file in "${proto_files[@]}"; do
    ((current++))
    filename=$(basename "$proto_file")
    print_info "[$current/$total_files] 处理: $filename"
    
    # 生成 protobuf 代码
    echo -n "  生成 protobuf... "
    if "$PROTOC" --proto_path="$PROTO_DIR" --cpp_out="$OUT_DIR" "$proto_file" 2>/tmp/protoc_error_pb_$$ >/dev/null; then
        echo "✓"
    else
        echo "✗"
        print_error "生成 protobuf 失败"
        cat /tmp/protoc_error_pb_$$
        failed_files+=("$filename (protobuf)")
        ((failed++))
        rm -f /tmp/protoc_error_pb_$$
        continue
    fi
    rm -f /tmp/protoc_error_pb_$$
    
    # 生成 gRPC 代码
    echo -n "  生成 gRPC... "
    if "$PROTOC" --proto_path="$PROTO_DIR" --grpc_out="$OUT_DIR" \
               --plugin=protoc-gen-grpc="$GRPC_PLUGIN" "$proto_file" 2>/tmp/protoc_error_grpc_$$ >/dev/null; then
        echo "✓"
    else
        echo "✗"
        print_error "生成 gRPC 失败"
        cat /tmp/protoc_error_grpc_$$
        failed_files+=("$filename (gRPC)")
        ((failed++))
        rm -f /tmp/protoc_error_grpc_$$
        continue
    fi
    rm -f /tmp/protoc_error_grpc_$$
done

if [ $failed -gt 0 ]; then
    print_error "有 $failed 个生成任务失败"
    print_info "失败的文件："
    for file in "${failed_files[@]}"; do
        echo "  - $file"
    done
    exit 1
fi

print_success "所有 proto 文件生成完成"

# 步骤 4：验证生成的文件
print_header "步骤 4/6: 验证生成的文件"

# 统计生成的文件
pb_h_count=$(ls -1 "$OUT_DIR"/*.pb.h 2>/dev/null | wc -l)
pb_cc_count=$(ls -1 "$OUT_DIR"/*.pb.cc 2>/dev/null | wc -l)
grpc_h_count=$(ls -1 "$OUT_DIR"/*.grpc.pb.h 2>/dev/null | wc -l)
grpc_cc_count=$(ls -1 "$OUT_DIR"/*.grpc.pb.cc 2>/dev/null | wc -l)

print_info "生成的文件统计："
echo "  - .pb.h:       $pb_h_count 个"
echo "  - .pb.cc:      $pb_cc_count 个"
echo "  - .grpc.pb.h:  $grpc_h_count 个"
echo "  - .grpc.pb.cc: $grpc_cc_count 个"

if [ $pb_h_count -eq 0 ]; then
    print_error "没有生成任何 .pb.h 文件！"
    ls -la "$OUT_DIR/"
    exit 1
fi

# 检查 common.pb.h 的版本信息
if [ -f "$OUT_DIR/common.pb.h" ]; then
    print_info "检查 common.pb.h 版本..."
    version_line=$(grep "PROTOBUF_VERSION <" "$OUT_DIR/common.pb.h" | head -n 1 || echo "未找到版本信息")
    echo "  $version_line"
    
    if echo "$version_line" | grep -q "4025000"; then
        print_success "版本正确：使用 protobuf 4.25.x"
    elif echo "$version_line" | grep -q "3021000"; then
        print_warning "版本仍是 protobuf 3.21.x"
        print_info "这可能导致编译错误，但会尝试继续..."
    else
        print_info "版本检测结果不确定"
    fi
else
    print_error "未找到 common.pb.h"
    exit 1
fi

# 步骤 5：清理并重新编译项目
print_header "步骤 5/6: 清理并重新编译项目"

print_info "删除旧的构建目录..."
rm -rf build

print_info "创建构建目录..."
mkdir build || { print_error "无法创建构建目录"; exit 1; }
cd build || { print_error "无法进入构建目录"; exit 1; }

print_info "运行 CMake 配置..."
if cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" \
    .. 2>&1 | tee /tmp/cmake_output_$$; then
    print_success "CMake 配置成功"
else
    print_error "CMake 配置失败"
    echo ""
    print_info "显示最后 30 行 CMake 输出："
    tail -n 30 /tmp/cmake_output_$$
    exit 1
fi

print_info "开始编译（可能需要几分钟）..."
echo ""
if ninja 2>&1 | tee /tmp/ninja_output_$$; then
    print_success "编译成功"
else
    print_error "编译失败"
    echo ""
    print_info "显示最后 50 行编译输出："
    tail -n 50 /tmp/ninja_output_$$
    echo ""
    print_info "完整输出保存在: /tmp/ninja_output_$$"
    exit 1
fi

cd ..

# 步骤 6：验证编译结果
print_header "步骤 6/6: 验证编译结果"

print_info "查找生成的可执行文件..."
executables=$(find build/src -type f -executable 2>/dev/null | grep -E "/(central|db|login|logic|gateway|battle|file|matching)$" || true)

if [ -z "$executables" ]; then
    print_warning "未找到可执行文件"
    print_info "构建目录内容："
    find build/src -type f 2>/dev/null | head -n 20
else
    exe_count=$(echo "$executables" | wc -l)
    print_success "找到 $exe_count 个可执行文件"
    
    echo ""
    print_info "可执行文件列表："
    while IFS= read -r exe; do
        if [ -n "$exe" ]; then
            size=$(ls -lh "$exe" 2>/dev/null | awk '{print $5}')
            name=$(basename "$exe")
            echo "  - $name ($size)"
        fi
    done <<< "$executables"
fi

# 完成
print_header "完成"

print_success "所有步骤已成功完成！"
echo ""
print_info "下一步："
echo "  1. 启动服务: bash manage_services.sh start"
echo "  2. 查看状态: bash manage_services.sh status"
echo ""

# 显示服务管理提示
if [ -f "manage_services.sh" ]; then
    print_info "可用的服务管理命令："
    echo "  bash manage_services.sh start    - 启动所有服务"
    echo "  bash manage_services.sh stop     - 停止所有服务"
    echo "  bash manage_services.sh status   - 查看运行状态"
    echo "  bash manage_services.sh restart  - 重启所有服务"
fi

echo ""
print_success "环境配置完成！祝你开发顺利！"

exit 0
