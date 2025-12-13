#!/bin/bash
# 项目服务统一管理脚本
# 功能：编译、启动、停止所有服务（包括 Skynet、C++ 服务器、数据库）

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_DIR="$PROJECT_ROOT/build"
SKYNET_DIR="$PROJECT_ROOT/skynet_src/skynet"
SKYNET_CONFIG="$PROJECT_ROOT/config/skynet/config.lua"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_header() {
    echo -e "\n${BLUE}===================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}===================================================${NC}\n"
}

print_success() { echo -e "${GREEN}[✓]${NC} $1"; }
print_error() { echo -e "${RED}[✗]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[!]${NC} $1"; }
print_info() { echo -e "${BLUE}[i]${NC} $1"; }

#==================== 编译相关 ====================#

# 生成 Protobuf 代码
generate_proto() {
    print_header "生成 Protobuf 代码"
    
    # C++ Proto
    print_info "生成 C++ Protobuf 代码..."
    PROTO_DIR="$PROJECT_ROOT/protobuf/cpp"
    OUT_DIR="$PROTO_DIR/src"
    
    PROTOC=""
    if command -v protoc &> /dev/null; then
        PROTOC="protoc"
    elif [ -f "$HOME/vcpkg/installed/x64-linux/tools/protobuf/protoc" ]; then
        PROTOC="$HOME/vcpkg/installed/x64-linux/tools/protobuf/protoc"
    else
        print_error "未找到 protoc"
        return 1
    fi
    
    mkdir -p "$OUT_DIR"
    cd "$PROTO_DIR"
    
    for proto_file in *.proto; do
        $PROTOC --cpp_out="$OUT_DIR" "$proto_file"
    done
    
    print_success "C++ Protobuf 代码生成完成"
    
    # Lua Proto
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
}

# 编译 Skynet
compile_skynet() {
    print_header "编译 Skynet"
    
    if [ ! -d "$SKYNET_DIR" ]; then
        print_error "Skynet 目录不存在: $SKYNET_DIR"
        return 1
    fi
    
    if [ -f "$SKYNET_DIR/skynet" ]; then
        print_success "Skynet 已编译"
        return 0
    fi
    
    print_info "正在编译 Skynet..."
    cd "$SKYNET_DIR"
    make linux
    
    if [ -f "$SKYNET_DIR/skynet" ]; then
        print_success "Skynet 编译完成"
    else
        print_error "Skynet 编译失败"
        return 1
    fi
}

# 编译 C++ 项目
compile_cpp() {
    print_header "编译 C++ 项目"
    
    local BUILD_TYPE=${1:-Release}
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    print_info "配置 CMake..."
    cmake -G Ninja \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
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

# 完整编译流程
compile_all() {
    print_header "完整编译流程"
    
    # 1. 生成 Proto
    generate_proto || return 1
    
    # 2. 编译 Skynet
    compile_skynet || return 1
    
    # 3. 编译 C++
    compile_cpp "${1:-Release}" || return 1
    
    print_header "编译完成"
    print_success "所有组件编译成功！"
}

#==================== 服务管理 ====================#

# 启动数据库
start_database() {
    print_info "启动数据库服务..."
    
    # Redis
    if ! redis-cli ping &>/dev/null; then
        print_info "启动 Redis..."
        sudo service redis-server start
        sleep 1
        if redis-cli ping &>/dev/null; then
            print_success "Redis 启动成功"
        else
            print_error "Redis 启动失败"
            return 1
        fi
    else
        print_success "Redis 已在运行"
    fi
}

# 停止数据库
stop_database() {
    print_info "停止数据库服务..."
    
    if sudo service redis-server stop 2>/dev/null; then
        print_success "Redis 已停止"
    fi
}

# 启动 Skynet
start_skynet() {
    print_info "启动 Skynet..."
    
    if [ ! -f "$SKYNET_DIR/skynet" ]; then
        print_error "Skynet 未编译"
        print_info "运行: bash $0 compile"
        return 1
    fi
    
    if pgrep -x "skynet" > /dev/null; then
        print_warning "Skynet 已在运行"
        return 0
    fi
    
    cd "$SKYNET_DIR"
    mkdir -p "$PROJECT_ROOT/logs"
    
    ./skynet "$SKYNET_CONFIG" > "$PROJECT_ROOT/logs/skynet.log" 2>&1 &
    local pid=$!
    sleep 2
    
    if ps -p $pid > /dev/null 2>&1; then
        print_success "Skynet 已启动 (PID: $pid)"
    else
        print_error "Skynet 启动失败"
        tail -20 "$PROJECT_ROOT/logs/skynet.log"
        return 1
    fi
}

# 停止 Skynet
stop_skynet() {
    if pgrep -x "skynet" > /dev/null; then
        print_info "停止 Skynet..."
        pkill -TERM "skynet"
        sleep 1
        
        if pgrep -x "skynet" > /dev/null; then
            pkill -KILL "skynet"
        fi
        
        print_success "Skynet 已停止"
    fi
}

# 启动 C++ 服务器
start_cpp_servers() {
    print_info "启动 C++ 服务器..."
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "构建目录不存在，请先编译"
        return 1
    fi
    
    local servers=("central" "db" "login" "gateway" "file" "matching")
    
    for server in "${servers[@]}"; do
        local service_path="$BUILD_DIR/src/${server}/${server}"
        
        if [ -f "$service_path" ]; then
            if pgrep -x "$server" > /dev/null; then
                print_warning "$server 已在运行"
                continue
            fi
            
            print_info "启动 $server..."
            cd "$BUILD_DIR/src/${server}"
            "./${server}" > "$PROJECT_ROOT/logs/${server}.log" 2>&1 &
            sleep 1
            
            if pgrep -x "$server" > /dev/null; then
                print_success "$server 启动成功"
            else
                print_error "$server 启动失败"
            fi
            cd - > /dev/null
        else
            print_warning "$server 不存在"
        fi
    done
}

# 停止 C++ 服务器
stop_cpp_servers() {
    print_info "停止 C++ 服务器..."
    
    local servers=("central" "db" "login" "gateway" "file" "matching")
    
    for server in "${servers[@]}"; do
        if pgrep -x "$server" > /dev/null; then
            print_info "停止 $server..."
            pkill -TERM "$server"
        fi
    done
    
    sleep 1
    
    for server in "${servers[@]}"; do
        if pgrep -x "$server" > /dev/null; then
            pkill -KILL "$server"
        fi
    done
}

# 启动所有服务
start_all() {
    print_header "启动所有服务"
    
    start_database || return 1
    sleep 1
    
    start_skynet || return 1
    sleep 2
    
    start_cpp_servers || return 1
    
    print_header "启动完成"
    print_success "所有服务已启动！"
}

# 停止所有服务
stop_all() {
    print_header "停止所有服务"
    
    stop_cpp_servers
    stop_skynet
    stop_database
    
    print_success "所有服务已停止"
}

# 查看状态
show_status() {
    print_header "服务状态"
    
    # Redis
    echo -n "Redis:     "
    if redis-cli ping &>/dev/null; then
        echo -e "${GREEN}运行中${NC}"
    else
        echo -e "${RED}未运行${NC}"
    fi
    
    # Skynet
    echo -n "Skynet:    "
    if pgrep -x "skynet" > /dev/null; then
        pid=$(pgrep -x "skynet")
        echo -e "${GREEN}运行中${NC} (PID: $pid)"
    else
        echo -e "${YELLOW}未运行${NC}"
    fi
    
    # C++ 服务器
    local servers=("central" "db" "login" "gateway" "file" "matching")
    for server in "${servers[@]}"; do
        echo -n "$(printf '%-10s' $server): "
        if pgrep -x "$server" > /dev/null; then
            pid=$(pgrep -x "$server")
            echo -e "${GREEN}运行中${NC} (PID: $pid)"
        else
            echo -e "${YELLOW}未运行${NC}"
        fi
    done
}

# 帮助信息
show_help() {
    cat << EOF
项目服务统一管理脚本

用法: bash $0 <命令> [参数]

编译命令:
  compile [Release|Debug]  - 完整编译流程（Proto + Skynet + C++）
  proto                    - 仅生成 Protobuf 代码
  skynet                   - 仅编译 Skynet
  cpp [Release|Debug]      - 仅编译 C++ 项目

服务管理命令:
  start                    - 启动所有服务（数据库 + Skynet + C++）
  stop                     - 停止所有服务
  restart                  - 重启所有服务
  status                   - 查看服务状态

示例:
  bash $0 compile           # 编译所有组件（Release 模式）
  bash $0 compile Debug     # 编译所有组件（Debug 模式）
  bash $0 start             # 启动所有服务
  bash $0 status            # 查看状态

EOF
}

# 主函数
main() {
    case "${1:-help}" in
        compile)
            compile_all "${2:-Release}"
            ;;
        proto)
            generate_proto
            ;;
        skynet)
            compile_skynet
            ;;
        cpp)
            compile_cpp "${2:-Release}"
            ;;
        start)
            start_all
            ;;
        stop)
            stop_all
            ;;
        restart)
            stop_all
            sleep 2
            start_all
            ;;
        status)
            show_status
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            print_error "未知命令: $1"
            show_help
            exit 1
            ;;
    esac
}

main "$@"
