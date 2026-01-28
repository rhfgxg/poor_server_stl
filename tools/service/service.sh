#!/bin/bash
# 服务管理脚本
# 启动、停止、重启、查看状态

set -e

# 加载公共函数
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

#==================== 数据库服务 ====================#

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

stop_database() {
    print_info "停止数据库服务..."
    
    if sudo service redis-server stop 2>/dev/null; then
        print_success "Redis 已停止"
    fi
}

#==================== Skynet 服务 ====================#

start_skynet() {
    print_info "启动 Skynet..."
    
    if ! file_exists "$SKYNET_DIR/skynet"; then
        print_error "Skynet 未编译，请先运行: bash build.sh"
        return 1
    fi
    
    if is_process_running "skynet"; then
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

stop_skynet() {
    if is_process_running "skynet"; then
        print_info "停止 Skynet..."
        pkill -TERM "skynet"
        sleep 1
        
        if is_process_running "skynet"; then
            pkill -KILL "skynet"
        fi
        
        print_success "Skynet 已停止"
    fi
}

#==================== C++ 服务器 ====================#

start_cpp_servers() {
    print_info "启动 C++ 服务器..."
    
    if ! dir_exists "$BUILD_DIR"; then
        print_error "构建目录不存在，请先运行: bash build.sh"
        return 1
    fi
    
    local servers=("central" "db" "login" "gateway" "file")
    
    for server in "${servers[@]}"; do
        local exec_name="$server"
        local process_name="$server"
        local service_path="$BUILD_DIR/src/${server}/${exec_name}"
        
        if file_exists "$service_path"; then
            if is_process_running "$process_name"; then
                print_warning "$server 已在运行"
                continue
            fi
            
            print_info "启动 $server..."
            local service_dir=$(dirname "$service_path")
            cd "$service_dir"
            "./${exec_name}" > "$PROJECT_ROOT/logs/${server}.log" 2>&1 &
            sleep 1
            
            if is_process_running "$process_name"; then
                print_success "$server 启动成功"
            else
                print_error "$server 启动失败"
                tail -10 "$PROJECT_ROOT/logs/${server}.log"
            fi
            cd - > /dev/null
        else
            print_warning "$server 不存在: $service_path"
        fi
    done
}

stop_cpp_servers() {
    print_info "停止 C++ 服务器..."
    
    local servers=("central" "db" "login" "gateway" "file")
    
    for server in "${servers[@]}"; do
        if is_process_running "$server"; then
            print_info "停止 $server..."
            pkill -TERM "$server"
        fi
    done
    
    sleep 1
    
    for server in "${servers[@]}"; do
        if is_process_running "$server"; then
            pkill -KILL "$server"
        fi
    done
    
    print_success "C++ 服务器已停止"
}

#==================== 服务状态 ====================#

show_status() {
    print_header "服务状态"
    
    echo "数据库:"
    echo -n "  Redis DB:      "
    if redis-cli ping &>/dev/null; then
        echo -e "${GREEN}运行中${NC}"
    else
        echo -e "${RED}未运行${NC}"
    fi
    
    echo ""
    echo "框架:"
    echo -n "  Skynet:        "
    if is_process_running "skynet"; then
        pid=$(pgrep -x "skynet")
        echo -e "${GREEN}运行中${NC} (PID: $pid)"
    else
        echo -e "${YELLOW}未运行${NC}"
    fi
    
    echo ""
    echo "C++ 服务器:"
    
    local servers=("central" "db" "login" "gateway" "file")
    local display_names=("central" "db" "login" "gateway" "file")
    
    for i in "${!servers[@]}"; do
        local server="${servers[$i]}"
        local display_name="${display_names[$i]}"
        
        echo -n "  $(printf '%-14s' $display_name): "
        if is_process_running "$server"; then
            pid=$(pgrep -x "$server")
            echo -e "${GREEN}运行中${NC} (PID: $pid)"
        else
            echo -e "${YELLOW}未运行${NC}"
        fi
    done
}

#==================== 主函数 ====================#

case "${1:-help}" in
    start)
        print_header "启动所有服务"
        start_database || exit 1
        sleep 1
        start_skynet || exit 1
        sleep 2
        start_cpp_servers || exit 1
        print_header "启动完成"
        print_success "所有服务已启动！"
        ;;
    stop)
        print_header "停止所有服务"
        stop_cpp_servers
        stop_skynet
        stop_database
        print_success "所有服务已停止"
        ;;
    restart)
        print_header "重启所有服务"
        stop_cpp_servers
        stop_skynet
        stop_database
        sleep 2
        start_database || exit 1
        sleep 1
        start_skynet || exit 1
        sleep 2
        start_cpp_servers || exit 1
        print_success "所有服务已重启"
        ;;
    status)
        show_status
        ;;
    start-db)
        start_database
        ;;
    stop-db)
        stop_database
        ;;
    start-skynet)
        start_skynet
        ;;
    stop-skynet)
        stop_skynet
        ;;
    start-cpp)
        start_cpp_servers
        ;;
    stop-cpp)
        stop_cpp_servers
        ;;
    *)
        cat << EOF
服务管理脚本

用法: bash $0 <命令>

服务控制:
  start          - 启动所有服务（数据库 + Skynet + C++）
  stop           - 停止所有服务
  restart        - 重启所有服务
  status         - 查看服务状态

分组控制:
  start-db       - 仅启动数据库
  stop-db        - 仅停止数据库
  start-skynet   - 仅启动 Skynet
  stop-skynet    - 仅停止 Skynet
  start-cpp      - 仅启动 C++ 服务器
  stop-cpp       - 仅停止 C++ 服务器

示例:
  bash $0 start          # 启动所有服务
  bash $0 status         # 查看状态
  bash $0 start-cpp      # 只启动 C++ 服务器

EOF
        ;;
esac
