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

# 查找 protoc（优先使用 vcpkg 版本）
PROTOC_PATHS=(
    "$HOME/vcpkg/installed/x64-linux/tools/protobuf/protoc"
    "$HOME/vcpkg/packages/protobuf_x64-linux/tools/protobuf/protoc"
)

for protoc_path in "${PROTOC_PATHS[@]}"; do
    if [ -f "$protoc_path" ]; then
        PROTOC="$protoc_path"
        print_info "使用 vcpkg protoc: $protoc_path"
        break
    fi
done

if [ -z "$PROTOC" ] && command_exists protoc; then
    PROTOC="protoc"
    print_warning "使用系统 protoc，可能版本不匹配"
fi

if [ -z "$PROTOC" ]; then
    print_error "未找到 protoc"
    exit 1
fi

# 查找 grpc_cpp_plugin（多个可能的路径）
GRPC_PLUGIN_PATHS=(
    "$HOME/vcpkg/installed/x64-linux/tools/grpc/grpc_cpp_plugin"
    "$HOME/vcpkg/packages/grpc_x64-linux/tools/grpc/grpc_cpp_plugin"
    "$HOME/vcpkg/buildtrees/grpc/x64-linux-rel/grpc_cpp_plugin"
)

for plugin_path in "${GRPC_PLUGIN_PATHS[@]}"; do
    if [ -f "$plugin_path" ]; then
        GRPC_PLUGIN="$plugin_path"
        print_info "找到 grpc_cpp_plugin: $plugin_path"
        break
    fi
done

if [ -z "$GRPC_PLUGIN" ]; then
    print_warning "未找到 grpc_cpp_plugin，将跳过 gRPC 代码生成"
    print_warning "请确保已安装 grpc: vcpkg install grpc"
fi

print_info "protoc: $PROTOC"
$PROTOC --version

#==================== C++ Proto ====================#

print_info "生成 C++ Protobuf 代码..."

PROTO_DIR="$PROJECT_ROOT/protobuf/cpp"
OUT_DIR="$PROTO_DIR/src"

mkdir -p "$OUT_DIR"

# 删除旧的生成文件（包括 .grpc.pb.* 文件）
print_info "删除旧的生成文件..."
rm -f "$OUT_DIR"/*.pb.h "$OUT_DIR"/*.pb.cc "$OUT_DIR"/*.grpc.pb.h "$OUT_DIR"/*.grpc.pb.cc

cd "$PROTO_DIR"

count=0
grpc_count=0

for proto_file in *.proto; do
    count=$((count + 1))
    print_info "  [$count] 处理: $proto_file"
    
    # 生成 protobuf
    $PROTOC --cpp_out="$OUT_DIR" "$proto_file"
    print_info "      ✓ 生成 .pb.cc/.pb.h"
    
    # 检查是否包含 service 定义
    if grep -q "^service " "$proto_file"; then
        # 生成 grpc（如果有 grpc_cpp_plugin）
        if [ -n "$GRPC_PLUGIN" ]; then
            $PROTOC --grpc_out="$OUT_DIR" \
                --plugin=protoc-gen-grpc="$GRPC_PLUGIN" \
                "$proto_file"
            print_info "      ✓ 生成 .grpc.pb.cc/.grpc.pb.h"
            grpc_count=$((grpc_count + 1))
        else
            print_warning "      ✗ 跳过 gRPC 生成（未找到 grpc_cpp_plugin）"
        fi
    fi
done

# 验证生成的文件
print_info "验证生成的文件..."
pb_count=$(ls -1 "$OUT_DIR"/*.pb.cc 2>/dev/null | wc -l)
grpc_pb_count=$(ls -1 "$OUT_DIR"/*.grpc.pb.cc 2>/dev/null | wc -l)

print_info "  .pb.cc 文件: $pb_count"
print_info "  .grpc.pb.cc 文件: $grpc_pb_count"

# 验证 REDIS 类型
if grep -q "REDIS = 11" "$OUT_DIR/common.pb.h" 2>/dev/null; then
    print_success "C++ Protobuf 代码生成完成 ($count 个 proto，$grpc_count 个 gRPC service，包含 REDIS 类型)"
else
    print_success "C++ Protobuf 代码生成完成 ($count 个 proto，$grpc_count 个 gRPC service)"
    print_warning "未找到 REDIS 类型"
fi

#==================== Lua Proto ====================#

print_info "生成 Lua Protobuf Descriptor..."

PROTO_DIR="$PROJECT_ROOT/protobuf/skynet"
OUT_DIR="$PROTO_DIR/src"

mkdir -p "$OUT_DIR"
cd "$PROTO_DIR"

lua_count=0
for proto_file in *.proto; do
    lua_count=$((lua_count + 1))
    desc_file="$OUT_DIR/${proto_file%.proto}.pb"
    print_info "  [$lua_count] 处理: $proto_file -> ${proto_file%.proto}.pb"
    $PROTOC --descriptor_set_out="$desc_file" --include_imports "$proto_file"
done

print_success "Lua Protobuf Descriptor 生成完成 ($lua_count 个文件)"

print_header "Proto 生成完成"
print_info "总结:"
print_info "  C++ Proto:      $pb_count 个 .pb.cc 文件"
print_info "  C++ gRPC:       $grpc_pb_count 个 .grpc.pb.cc 文件"
print_info "  Lua Descriptor: $lua_count 个 .pb 文件"
