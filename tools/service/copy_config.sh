#!/bin/bash
# 配置文件复制脚本
# 将配置文件从 config/ 复制到 build/ 对应的目录

set -e

# 加载公共函数
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

print_header "复制配置文件"

if ! dir_exists "$BUILD_DIR"; then
    print_warning "构建目录不存在，跳过配置复制"
    exit 0
fi

CONFIG_DIR="$PROJECT_ROOT/config/cpp/cfg_server"

# 定义服务器配置映射
declare -A SERVER_CONFIGS=(
    ["central"]="cfg_central_server.lua"
    ["db"]="cfg_db_server.lua"
    ["file"]="cfg_file_server.lua"
    ["gateway"]="cfg_gateway_server.lua"
    ["login"]="cfg_login_server.lua"
    ["matching"]="cfg_matching_server.lua"
    ["redis"]="cfg_redis.lua"
)

SUCCESS_COUNT=0

# 复制各服务器配置文件
for server in "${!SERVER_CONFIGS[@]}"; do
    config_file="${SERVER_CONFIGS[$server]}"
    target_dir="$BUILD_DIR/src/$server/config"
    source_file="$CONFIG_DIR/$config_file"
    target_file="$target_dir/$config_file"
    
    mkdir -p "$target_dir"
    
    if file_exists "$source_file"; then
        if cp "$source_file" "$target_file" 2>/dev/null; then
            print_info "  ✓ $server: $config_file"
            SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
        else
            print_warning "  ✗ $server: $config_file (复制失败)"
        fi
    else
        print_warning "  - $server: $config_file (源文件不存在)"
    fi
done

# 复制额外的数据库配置
DB_CONFIG="cfg_db.lua"
DB_SOURCE="$CONFIG_DIR/$DB_CONFIG"
DB_TARGET="$BUILD_DIR/src/db/config/$DB_CONFIG"

if file_exists "$DB_SOURCE"; then
    mkdir -p "$(dirname "$DB_TARGET")"
    if cp "$DB_SOURCE" "$DB_TARGET" 2>/dev/null; then
        print_info "  ✓ db: $DB_CONFIG (额外)"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    fi
fi

print_success "配置文件复制完成 (成功: $SUCCESS_COUNT 个文件)"
