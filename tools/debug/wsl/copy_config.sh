#!/bin/bash
# 配置文件复制脚本（Linux/WSL2）
# 将配置文件从 config/cpp/cfg_server 复制到各服务器的 build 目录

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# 项目根目录（从 tools/debug/wsl 向上三级）
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
# 源配置目录
CONFIG_SRC="$PROJECT_ROOT/config/cpp/cfg_server"
# 构建目录
BUILD_DIR="$PROJECT_ROOT/build"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# 复制配置文件的通用函数
copy_config() {
    local config_file=$1
    local target_dir=$2
    local config_path="$CONFIG_SRC/$config_file"
    
    # 检查源文件是否存在
    if [ ! -f "$config_path" ]; then
        print_warning "配置文件不存在: $config_path"
        return 1
    fi
    
    # 创建目标目录
    mkdir -p "$target_dir"
    
    # 复制文件
    if cp "$config_path" "$target_dir/"; then
        print_success "已复制: $config_file → $target_dir"
        return 0
    else
        print_error "复制失败: $config_file"
        return 1
    fi
}

# 主函数
main() {
    print_info "开始复制配置文件..."
    echo ""
    
    # 检查源配置目录
    if [ ! -d "$CONFIG_SRC" ]; then
        print_error "配置源目录不存在: $CONFIG_SRC"
        exit 1
    fi
    
    # 检查构建目录
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "构建目录不存在: $BUILD_DIR"
        print_info "请先编译项目: cd $PROJECT_ROOT && bash build.sh Release"
        exit 1
    fi
    
    print_info "源配置目录: $CONFIG_SRC"
    print_info "构建目录:   $BUILD_DIR"
    echo ""
    
    local success_count=0
    local fail_count=0
    
    # 定义服务器与配置文件的映射
    declare -A server_configs=(
        ["central"]="cfg_central_server.lua"
        ["db"]="cfg_db_server.lua"
        ["login"]="cfg_login_server.lua"
        ["logic"]="cfg_logic_server.lua"
        ["gateway"]="cfg_gateway_server.lua"
        ["battle"]="cfg_battle_server.lua"
        ["file"]="cfg_file_server.lua"
        ["matching"]="cfg_matching_server.lua"
    )
    
    # 复制每个服务器的配置
    for server in "${!server_configs[@]}"; do
        local config_file="${server_configs[$server]}"
        local server_build_dir="$BUILD_DIR/src/$server"
        
        # 检查服务器构建目录是否存在
        if [ ! -d "$server_build_dir" ]; then
            print_warning "服务器目录不存在: $server_build_dir (跳过)"
            continue
        fi
        
        # 创建 config 子目录
        local config_dir="$server_build_dir/config"
        
        if copy_config "$config_file" "$config_dir"; then
            ((success_count++))
        else
            ((fail_count++))
        fi
    done
    
    # 额外复制 cfg_db.lua 到 db 服务器
    if [ -d "$BUILD_DIR/src/db" ]; then
        if copy_config "cfg_db.lua" "$BUILD_DIR/src/db/config"; then
            ((success_count++))
        else
            ((fail_count++))
        fi
    fi
    
    echo ""
    print_info "===================="
    print_success "成功: $success_count 个文件"
    if [ $fail_count -gt 0 ]; then
        print_error "失败: $fail_count 个文件"
    fi
    print_info "===================="
    
    echo ""
    print_info "配置文件已复制到各服务器的 config 目录"
    print_info "例如: $BUILD_DIR/src/central/config/cfg_central_server.lua"
    echo ""
    print_info "如果修改了配置文件，需要重新运行此脚本"
}

main "$@"
