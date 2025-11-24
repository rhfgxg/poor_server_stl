#!/bin/bash
# Generate Protobuf Descriptor Files for Skynet
# 生成 Protobuf Descriptor 文件供 Skynet 使用

# 检查 protoc 是否安装
if ! command -v protoc &> /dev/null; then
    echo "Error: protoc not found!"
    echo "Please install Protocol Buffers compiler:"
    echo "  - Ubuntu/Debian: sudo apt-get install protobuf-compiler"
    echo "  - CentOS/RHEL: sudo yum install protobuf-compiler"
    echo "  - macOS: brew install protobuf"
    exit 1
fi

echo "Found protoc: $(which protoc)"
echo "Version: $(protoc --version)"

# 设置路径
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PROTO_DIR="$PROJECT_ROOT/protobuf/skynet"
DESC_DIR="$PROTO_DIR/src"

# 创建输出目录
mkdir -p "$DESC_DIR"
echo "Created directory: $DESC_DIR"

# 获取所有 proto 文件
PROTO_FILES=("$PROTO_DIR"/*.proto)

if [ ${#PROTO_FILES[@]} -eq 0 ]; then
    echo "No .proto files found in $PROTO_DIR"
    exit 0
fi

echo ""
echo "Generating descriptor files..."
echo "Proto directory: $PROTO_DIR"
echo "Output directory: $DESC_DIR"
echo ""

SUCCESS_COUNT=0
FAIL_COUNT=0

# 生成每个 proto 文件的 descriptor
for PROTO_FILE in "${PROTO_FILES[@]}"; do
    if [ ! -f "$PROTO_FILE" ]; then
        continue
    fi
    
    PROTO_NAME=$(basename "$PROTO_FILE" .proto)
    DESC_PATH="$DESC_DIR/$PROTO_NAME.pb"
    
    echo -n "Processing: $(basename "$PROTO_FILE")..."
    
    # 执行 protoc 命令
    if protoc --descriptor_set_out="$DESC_PATH" \
              --include_imports \
              --proto_path="$PROTO_DIR" \
              "$PROTO_FILE" 2>/tmp/protoc_error.txt; then
        echo " OK"
        ((SUCCESS_COUNT++))
    else
        echo " FAILED"
        echo "  Error: $(cat /tmp/protoc_error.txt)"
        ((FAIL_COUNT++))
    fi
done

echo ""
echo "Summary:"
echo "  Success: $SUCCESS_COUNT"
echo "  Failed:  $FAIL_COUNT"
echo "  Output:  $DESC_DIR"

if [ $SUCCESS_COUNT -gt 0 ]; then
    echo ""
    echo "Generated descriptor files:"
    ls -lh "$DESC_DIR"/*.pb 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
fi

echo ""
echo "Done!"
