#!/bin/bash
# Proto 代码生成脚本
# 生成 C++ 和 Lua 的 Protobuf 代码

set -e

# 加载公共函数
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

print_header "生成 Protobuf 代码"

# 查找 protoc 和 grpc_cpp_plugin
PROTOC=""
GRPC_PLUGIN=""

# 优先使用 vcpkg 版本
if [ -f "$HOME/vcpkg/installed/x64-linux/tools/protobuf/protoc" ]; then
    PROTOC="$HOME/vcpkg/installed/x64-linux/tools/protobuf/protoc"
    GRPC_PLUGIN="$HOME/vcpkg/installed/x64-linux/tools/grpc/grpc_cpp_plugin"
    print_info "使用 vcpkg protoc"
elif command_exists protoc; then
    PROTOC="protoc"
    print_warning "使用系统 protoc，可能版本不匹配"
else
    print_error "未找到 protoc"
    exit 1
fi

print_info "protoc: $PROTOC"
$PROTOC --version

#==================== C++ Proto ====================#

print_info "生成 C++ Protobuf 代码..."

PROTO_DIR="$PROJECT_ROOT/protobuf/cpp"
OUT_DIR="$PROTO_DIR/src"

mkdir -p "$OUT_DIR"

# 删除旧的生成文件
print_info "删除旧的生成文件..."
rm -f "$OUT_DIR"/*.pb.h "$OUT_DIR"/*.pb.cc

cd "$PROTO_DIR"

count=0
for proto_file in *.proto; do
    count=$((count + 1))
    print_info "  [$count] 处理: $proto_file"
    
    # 生成 protobuf
    $PROTOC --cpp_out="$OUT_DIR" "$proto_file"
    
    # 生成 grpc（如果有 grpc_cpp_plugin）
    if [ -n "$GRPC_PLUGIN" ] && [ -f "$GRPC_PLUGIN" ]; then
        $PROTOC --grpc_out="$OUT_DIR" \
            --plugin=protoc-gen-grpc="$GRPC_PLUGIN" \
            "$proto_file"
    fi
done

# 验证 REDIS 类型
if grep -q "REDIS = 11" "$OUT_DIR/common.pb.h" 2>/dev/null; then
    print_success "C++ Protobuf 代码生成完成 ($count 个文件，包含 REDIS 类型)"
else
    print_success "C++ Protobuf 代码生成完成 ($count 个文件)"
    print_warning "未找到 REDIS 类型"
fi

#==================== Lua Proto ====================#

print_info "生成 Lua Protobuf Descriptor..."

PROTO_DIR="$PROJECT_ROOT/protobuf/skynet"
OUT_DIR="$PROTO_DIR/src"

mkdir -p "$OUT_DIR"
cd "$PROTO_DIR"

for proto_file in *.proto; do
    desc_file="$OUT_DIR/${proto_file%.proto}.pb"
    $PROTOC --descriptor_set_out="$desc_file" --include_imports "$proto_file"
done

print_success "Lua Protobuf Descriptor 生成完成"

print_header "Proto 生成完成"
