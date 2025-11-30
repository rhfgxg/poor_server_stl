#!/bin/bash
# 批量修复文件编码和换行符（Linux/WSL 版本）
# 用途: 将项目中所有源代码文件统一为 UTF-8 无 BOM + LF 换行
# 使用: bash tools/debug/wsl/fix_encoding_and_lineendings.sh [--dry-run]

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

DRY_RUN=false
if [ "$1" == "--dry-run" ]; then
    DRY_RUN=true
fi

echo -e "${CYAN}=====================================${NC}"
echo -e "${CYAN}  文件编码和换行符修复工具${NC}"
echo -e "${CYAN}=====================================${NC}"
echo ""

if [ "$DRY_RUN" == true ]; then
    echo -e "${YELLOW}[预览模式] 不会实际修改文件${NC}"
    echo ""
fi

# 检查依赖工具
check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}✗ 错误: 未找到工具 $1${NC}"
        echo -e "${YELLOW}  安装方法: sudo apt install $2${NC}"
        exit 1
    fi
}

echo "检查依赖工具..."
check_tool "dos2unix" "dos2unix"
check_tool "file" "file"
echo -e "${GREEN}✓ 依赖工具检查通过${NC}"
echo ""

# 需要处理的文件扩展名
FILE_PATTERNS=(
    "*.cpp" "*.h" "*.hpp" "*.cc" "*.cxx" "*.hxx" "*.c" "*.inl"  # C++
    "*.lua"                                                       # Lua
    "*.cmake"                                                     # CMake
    "*.proto"                                                     # Protobuf
    "*.json" "*.yaml" "*.yml"                                    # 配置
    "*.md" "*.txt"                                               # 文档
)

# 排除目录
EXCLUDE_DIRS=(
    "build"
    "out"
    "vcpkg_installed"
    ".vs"
    ".vscode"
    ".git"
    "node_modules"
)

# 构建 find 命令的排除参数
EXCLUDE_ARGS=""
for dir in "${EXCLUDE_DIRS[@]}"; do
    EXCLUDE_ARGS="$EXCLUDE_ARGS -path ./$dir -prune -o"
done

TOTAL_FILES=0
MODIFIED_FILES=0
ERROR_FILES=0

# 处理每种文件类型
for pattern in "${FILE_PATTERNS[@]}"; do
    echo "正在处理 $pattern 文件..."
    
    # 使用 find 查找文件，排除指定目录
    while IFS= read -r file; do
        if [ ! -f "$file" ]; then
            continue
        fi
        
        TOTAL_FILES=$((TOTAL_FILES + 1))
        NEEDS_CHANGE=false
        CHANGE_REASON=""
        
        # 检测 BOM
        if file "$file" | grep -q "UTF-8 Unicode (with BOM)"; then
            NEEDS_CHANGE=true
            CHANGE_REASON="$CHANGE_REASON [BOM]"
        fi
        
        # 检测非 UTF-8 编码
        if ! file "$file" | grep -qE "UTF-8|ASCII"; then
            NEEDS_CHANGE=true
            ENCODING=$(file -b --mime-encoding "$file")
            CHANGE_REASON="$CHANGE_REASON [编码: $ENCODING]"
        fi
        
        # 检测 CRLF
        if file "$file" | grep -q "CRLF"; then
            NEEDS_CHANGE=true
            CHANGE_REASON="$CHANGE_REASON [CRLF]"
        fi
        
        if [ "$NEEDS_CHANGE" == true ]; then
            MODIFIED_FILES=$((MODIFIED_FILES + 1))
            echo -e "${YELLOW}[$TOTAL_FILES] 需要修复: $file${NC}"
            echo -e "    ${CHANGE_REASON}"
            
            if [ "$DRY_RUN" == false ]; then
                # 转换为 UTF-8
                if ! file "$file" | grep -q "UTF-8"; then
                    iconv -f GB2312 -t UTF-8 "$file" -o "$file.tmp" 2>/dev/null || \
                    iconv -f GBK -t UTF-8 "$file" -o "$file.tmp" 2>/dev/null || \
                    cp "$file" "$file.tmp"
                    mv "$file.tmp" "$file"
                fi
                
                # 移除 BOM
                if file "$file" | grep -q "with BOM"; then
                    sed -i '1s/^\xEF\xBB\xBF//' "$file"
                fi
                
                # 转换换行符为 LF
                dos2unix "$file" 2>/dev/null || true
                
                echo -e "    ${GREEN}✓ 已修复${NC}"
            fi
        else
            echo -e "${GREEN}[$TOTAL_FILES] ✓ 已符合规范: $file${NC}"
        fi
    done < <(eval "find . $EXCLUDE_ARGS -type f -name '$pattern' -print")
done

echo ""
echo -e "${CYAN}=====================================${NC}"
echo -e "${CYAN}  处理完成${NC}"
echo -e "${CYAN}=====================================${NC}"
echo "总文件数: $TOTAL_FILES"
echo -e "${YELLOW}需要修复: $MODIFIED_FILES${NC}"
echo -e "${RED}处理失败: $ERROR_FILES${NC}"
echo -e "${GREEN}已符合规范: $((TOTAL_FILES - MODIFIED_FILES - ERROR_FILES))${NC}"

if [ "$DRY_RUN" == true ] && [ $MODIFIED_FILES -gt 0 ]; then
    echo ""
    echo -e "${YELLOW}这是预览模式，没有实际修改文件。${NC}"
    echo -e "${YELLOW}要实际修复，请运行: bash tools/fix_encoding_and_lineendings.sh${NC}"
fi
