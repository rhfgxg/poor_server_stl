#!/bin/bash
# Generate C++ Protobuf and gRPC code
# 根据 proto 文件，生成 C++ 对应源码

# 设置路径常量
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
PROTO_DIR="$PROJECT_DIR/protobuf/cpp"
MAKE_OUT="$PROJECT_DIR/protobuf/cpp/src"

# 查找 protoc（尝试多个可能的位置）
if command -v protoc &> /dev/null; then
    PROTOC="protoc"
elif [ -f "$PROJECT_DIR/vcpkg_installed/x64-linux/tools/protobuf/protoc" ]; then
    PROTOC="$PROJECT_DIR/vcpkg_installed/x64-linux/tools/protobuf/protoc"
elif [ -f "$PROJECT_DIR/vcpkg_installed/x64-osx/tools/protobuf/protoc" ]; then
    PROTOC="$PROJECT_DIR/vcpkg_installed/x64-osx/tools/protobuf/protoc"
else
    echo "Error: protoc not found!"
    echo "Please install Protocol Buffers compiler:"
    echo "  - Ubuntu/Debian: sudo apt-get install protobuf-compiler"
    echo "  - CentOS/RHEL: sudo yum install protobuf-compiler"
    echo "  - macOS: brew install protobuf"
    exit 1
fi

# 查找 grpc_cpp_plugin
if [ -f "$PROJECT_DIR/vcpkg_installed/x64-linux/tools/grpc/grpc_cpp_plugin" ]; then
    GRPC_PLUGIN="$PROJECT_DIR/vcpkg_installed/x64-linux/tools/grpc/grpc_cpp_plugin"
elif [ -f "$PROJECT_DIR/vcpkg_installed/x64-osx/tools/grpc/grpc_cpp_plugin" ]; then
    GRPC_PLUGIN="$PROJECT_DIR/vcpkg_installed/x64-osx/tools/grpc/grpc_cpp_plugin"
elif command -v grpc_cpp_plugin &> /dev/null; then
    GRPC_PLUGIN="grpc_cpp_plugin"
else
    echo "Error: grpc_cpp_plugin not found!"
    echo "Please install gRPC:"
    echo "  - Ubuntu/Debian: sudo apt-get install libgrpc++-dev"
    echo "  - macOS: brew install grpc"
    exit 1
fi

echo "Using protoc: $PROTOC"
echo "Using gRPC plugin: $GRPC_PLUGIN"

# 创建输出目录
mkdir -p "$MAKE_OUT"

echo ""
echo "Generating C++ protobuf and gRPC code..."
echo "Proto directory: $PROTO_DIR"
echo "Output directory: $MAKE_OUT"
echo ""

# 辅助函数：生成 protobuf 和 gRPC 代码
generate_proto_module() {
    local module_name=$1
    local description=$2
    local is_commented=${3:-false}
    
    local proto_file="$PROTO_DIR/${module_name}.proto"
    
    if [ "$is_commented" = "true" ]; then
        echo "Skipped: $module_name ($description) - commented out"
        return
    fi
    
    if [ ! -f "$proto_file" ]; then
        echo "Skipped: $module_name - file not found"
        return
    fi
    
    echo "Processing: $module_name - $description"
    
    # 生成 protobuf 代码
    echo -n "  Generating protobuf code..."
    if $PROTOC --proto_path="$PROTO_DIR" --cpp_out="$MAKE_OUT" "$proto_file" 2>/tmp/protoc_error.txt; then
        echo " OK"
    else
        echo " FAILED"
        cat /tmp/protoc_error.txt
        return
    fi
    
    # 生成 gRPC 代码
    echo -n "  Generating gRPC code..."
    if $PROTOC --proto_path="$PROTO_DIR" --grpc_out="$MAKE_OUT" \
               --plugin=protoc-gen-grpc="$GRPC_PLUGIN" "$proto_file" 2>/tmp/protoc_error.txt; then
        echo " OK"
    else
        echo " FAILED"
        cat /tmp/protoc_error.txt
    fi
}

# 生成各模块代码
generate_proto_module "common" "包含共享数据类型，枚举的定义"
generate_proto_module "server_chat" "聊天服务器" "true"
generate_proto_module "server_central" "中心服务器"
generate_proto_module "server_db" "数据库服务器"
generate_proto_module "server_file" "文件服务器"
generate_proto_module "server_gateway" "网关服务器"
generate_proto_module "server_login" "登录服务器"
generate_proto_module "server_matching" "匹配服务器"
generate_proto_module "server_store" "商城服务器" "true"

echo ""
echo "脚本执行完毕"
echo "生成的文件位于: $MAKE_OUT"

# 显示生成的文件
echo ""
echo "生成的文件:"
ls -lh "$MAKE_OUT"/*.pb.* 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'

echo ""
read -p "Press Enter to continue..."
