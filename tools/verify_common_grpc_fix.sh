#!/bin/bash
# 验证 common.grpc.pb 修复脚本
# 检查是否还有错误的引用

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "======================================"
echo "验证 common.grpc.pb 修复"
echo "======================================"
echo ""

# 1. 检查 CMakeLists.txt 中是否还有 common.grpc.pb.cc
echo "[1/4] 检查 CMakeLists.txt 文件..."
FOUND=0
for cmake_file in $(find "$PROJECT_ROOT/src" -name "CMakeLists.txt"); do
    if grep -q "common.grpc.pb.cc" "$cmake_file"; then
        echo "  ❌ 错误: $cmake_file 仍然包含 common.grpc.pb.cc"
        FOUND=1
    fi
done

if [ $FOUND -eq 0 ]; then
    echo "  ✅ 所有 CMakeLists.txt 文件正确"
else
    echo "  ⚠️  请移除 CMakeLists.txt 中的 common.grpc.pb.cc"
    exit 1
fi

# 2. 检查 C++ 头文件中是否还有 common.grpc.pb.h
echo ""
echo "[2/4] 检查 C++ 头文件..."
FOUND=0
for header_file in $(find "$PROJECT_ROOT/src" -name "*.h" -o -name "*.hpp"); do
    if grep -q '#include.*"common.grpc.pb.h"' "$header_file" || \
       grep -q '#include.*<common.grpc.pb.h>' "$header_file"; then
        echo "  ❌ 错误: $header_file 仍然包含 common.grpc.pb.h"
        FOUND=1
    fi
done

if [ $FOUND -eq 0 ]; then
    echo "  ✅ 所有头文件正确"
else
    echo "  ⚠️  请将 #include \"common.grpc.pb.h\" 改为 #include \"common.pb.h\""
    exit 1
fi

# 3. 检查是否存在不应该存在的文件
echo ""
echo "[3/4] 检查生成的文件..."
PROTO_SRC_DIR="$PROJECT_ROOT/protobuf/cpp/src"

if [ -f "$PROTO_SRC_DIR/common.grpc.pb.cc" ] || [ -f "$PROTO_SRC_DIR/common.grpc.pb.h" ]; then
    echo "  ⚠️  发现不应该存在的文件:"
    [ -f "$PROTO_SRC_DIR/common.grpc.pb.cc" ] && echo "    - common.grpc.pb.cc"
    [ -f "$PROTO_SRC_DIR/common.grpc.pb.h" ] && echo "    - common.grpc.pb.h"
    echo ""
    echo "  建议删除这些文件:"
    echo "    rm -f $PROTO_SRC_DIR/common.grpc.pb.*"
else
    echo "  ✅ 没有多余的文件"
fi

# 4. 检查应该存在的文件
echo ""
echo "[4/4] 检查必需的文件..."
if [ -f "$PROTO_SRC_DIR/common.pb.cc" ] && [ -f "$PROTO_SRC_DIR/common.pb.h" ]; then
    echo "  ✅ common.pb.cc 和 common.pb.h 存在"
else
    echo "  ❌ 缺少必需的文件"
    echo "  请运行 proto 生成脚本:"
    echo "    bash tools/service/generate_proto.sh"
    exit 1
fi

echo ""
echo "======================================"
echo "✅ 验证完成！所有检查通过"
echo "======================================"
echo ""
echo "下一步:"
echo "  1. 如果有不应该存在的文件，请删除它们"
echo "  2. 运行编译: bash tools/service/manage.sh compile"
echo ""
