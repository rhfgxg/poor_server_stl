#!/bin/bash
# 项目服务管理脚本
# 用于启动、停止、重启项目的所有服务

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# 项目根目录（从 tools/debug/wsl 向上三级）
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
# 构建目录
BUILD_DIR="$PROJECT_ROOT/build"
# 配置复制脚本
COPY_CONFIG_SCRIPT="$SCRIPT_DIR/copy_config.sh"
# Skynet 目录
SKYNET_DIR="$PROJECT_ROOT/skynet_src/skynet"
SKYNET_CONFIG="$PROJECT_ROOT/config/skynet/config.lua"

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

# 检查 Skynet 是否已编译
check_skynet_compiled() {
    if [ ! -f "$SKYNET_DIR/skynet" ]; then
        print_error "Skynet 未编译！"
        print_info "请运行: cd $SKYNET_DIR && make linux"
        return 1
    fi
    return 0
}

# 启动 Skynet
start_skynet() {
    print_info "启动 Skynet 逻辑服务器..."
    
    # 检查 Skynet 是否已编译
    if ! check_skynet_compiled; then
        return 1
    fi
    
    # 检查配置文件
    if [ ! -f "$SKYNET_CONFIG" ]; then
        print_error "Skynet 配置文件不存在: $SKYNET_CONFIG"
        return 1
    fi
    
    # 检查是否已在运行
    if pgrep -x "skynet" > /dev/null; then
        print_warning "Skynet 已在运行"
        return 0
    fi
    
    # 切换到 Skynet 目录
    cd "$SKYNET_DIR" || {
        print_error "无法切换到 Skynet 目录: $SKYNET_DIR"
        return 1
    }
    
    # 计算相对配置文件路径
    RELATIVE_CONFIG="../../config/skynet/config.lua"
    
    # 检查相对路径是否正确
    if [ ! -f "$RELATIVE_CONFIG" ]; then
        print_error "从 Skynet 目录找不到配置文件: $RELATIVE_CONFIG"
        print_info "当前目录: $(pwd)"
        print_info "尝试的配置路径: $RELATIVE_CONFIG"
        cd - > /dev/null
        return 1
    fi
    
    # 创建日志目录
    mkdir -p "$PROJECT_ROOT/logs"
    
    # 启动 Skynet（输出到日志文件）
    print_info "启动命令: ./skynet $RELATIVE_CONFIG"
    ./skynet "$RELATIVE_CONFIG" > "$PROJECT_ROOT/logs/skynet.log" 2>&1 &
    local pid=$!
    sleep 2
    
    # 检查进程是否还在运行
    if ps -p $pid > /dev/null 2>&1; then
        print_success "Skynet 已启动 (PID: $pid)"
        print_info "日志文件: $PROJECT_ROOT/logs/skynet.log"
        cd - > /dev/null
        return 0
    else
        print_error "Skynet 启动失败"
        print_info "请查看日志: tail -20 $PROJECT_ROOT/logs/skynet.log"
        echo ""
        print_info "最近的错误信息:"
        tail -20 "$PROJECT_ROOT/logs/skynet.log"
        echo ""
        print_info "诊断建议:"
        print_info "  1. 检查 Skynet 是否已编译: ls -lh $SKYNET_DIR/skynet"
        print_info "  2. 检查服务文件是否存在: ls $PROJECT_ROOT/skynet_src/service/main.lua"
        print_info "  3. 运行诊断脚本: bash $PROJECT_ROOT/tools/skynet/diagnose_skynet.sh"
        cd - > /dev/null
        return 1
    fi
}

# 停止 Skynet
stop_skynet() {
    if pgrep -x "skynet" > /dev/null; then
        print_info "停止 Skynet 服务器..."
        pkill -TERM "skynet"
        sleep 1
        
        # 检查是否还在运行
        if pgrep -x "skynet" > /dev/null; then
            print_warning "Skynet 未响应，强制停止..."
            pkill -KILL "skynet"
            sleep 0.5
        fi
        
        # 最终检查
        if ! pgrep -x "skynet" > /dev/null; then
            print_success "Skynet 已停止"
        else
            print_error "Skynet 停止失败"
        fi
    fi
}

# 检查 Redis 是否运行
check_redis() {
    if redis-cli ping &>/dev/null; then
        print_success "Redis 正在运行"
        return 0
    else
        print_error "Redis 未运行！"
        print_info "启动 Redis: sudo service redis-server start"
        return 1
    fi
}

# 检查 MySQL 是否已安装
check_mysql_installed() {
    if systemctl list-unit-files 2>/dev/null | grep -q "^mysql.service"; then
        return 0
    elif [ -f "/etc/init.d/mysql" ]; then
        return 0
    else
        return 1
    fi
}

# 复制配置文件
copy_configs() {
    print_info "准备配置文件..."
    
    # 检查配置复制脚本是否存在
    if [ ! -f "$COPY_CONFIG_SCRIPT" ]; then
        print_error "配置复制脚本不存在: $COPY_CONFIG_SCRIPT"
        return 1
    fi
    
    # 运行配置复制脚本
    if bash "$COPY_CONFIG_SCRIPT"; then
        return 0
    else
        print_error "配置文件复制失败"
        return 1
    fi
}

# 启动数据库服务
start_databases() {
    print_info "启动数据库服务..."
    
    # 启动 Redis
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
    
    # 启动 MySQL（仅当已安装时）
    if command -v mysql &>/dev/null; then
        if check_mysql_installed; then
            if ! sudo service mysql status &>/dev/null 2>&1; then
                print_info "启动 MySQL..."
                if sudo service mysql start &>/dev/null 2>&1; then
                    sleep 1
                    print_success "MySQL 启动成功"
                else
                    print_warning "MySQL 启动失败或未安装"
                    print_info "如果使用 Windows 的 MySQL，请忽略此警告"
                fi
            else
                print_success "MySQL 已在运行"
            fi
        else
            print_info "MySQL 服务未安装（可能使用 Windows 的 MySQL）"
        fi
    fi
}

# 停止数据库服务
stop_databases() {
    print_info "停止数据库服务..."
    
    # 停止 Redis
    if sudo service redis-server stop 2>/dev/null; then
        print_success "Redis 已停止"
    else
        print_info "Redis 未运行或停止失败"
    fi
    
    # 停止 MySQL（仅当已安装时）
    if check_mysql_installed; then
        if sudo service mysql stop 2>/dev/null; then
            print_success "MySQL 已停止"
        else
            print_info "MySQL 未运行或停止失败"
        fi
    fi
}

# 启动所有服务器
start_servers() {
    print_info "启动项目服务器..."
    echo ""
    
    # 检查 Redis
    if ! check_redis; then
        print_error "请先启动 Redis！"
        print_info "运行: sudo service redis-server start"
        print_info "或运行: bash $0 start-db"
        return 1
    fi
    
    # 检查构建目录
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "构建目录不存在: $BUILD_DIR"
        print_info "请先编译项目:"
        print_info "  cd $PROJECT_ROOT"
        print_info "  bash build.sh Release"
        return 1
    fi
    
    # 复制配置文件
    echo ""
    if ! copy_configs; then
        print_warning "配置文件复制失败，服务器可能无法启动"
        read -p "是否继续启动服务器？(y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            return 1
        fi
    fi
    echo ""
    
    # 先启动 Skynet
    start_skynet
    sleep 2
    
    # 启动服务器的通用函数
    start_service() {
        local service_name=$1
        local service_path="$BUILD_DIR/src/${service_name}/${service_name}"
        
        if [ -f "$service_path" ]; then
            print_info "启动 ${service_name^} 服务器..."
            
            # 切换到服务器目录（以便正确加载配置文件）
            cd "$BUILD_DIR/src/${service_name}" || return 1
            
            # 后台启动服务器
            "./${service_name}" &
            local pid=$!
            sleep 1
            
            # 检查进程是否还在运行
            if ps -p $pid > /dev/null 2>&1; then
                print_success "${service_name^} 服务器已启动 (PID: $pid)"
                # 切回原目录
                cd - > /dev/null
                return 0
            else
                print_error "${service_name^} 服务器启动失败"
                # 切回原目录
                cd - > /dev/null
                return 1
            fi
        else
            print_warning "${service_name^} 服务器不存在: $service_path"
            return 1
        fi
    }
    
    # 按顺序启动各个服务器
    start_service "central"
    sleep 1
    
    start_service "db"
    sleep 1
    
    start_service "login"
    sleep 1
    
    start_service "gateway"
    sleep 1
    
    start_service "file"
    sleep 1
    
    start_service "matching"
    
    echo ""
    print_success "服务器启动完成！"
    echo ""
    print_info "查看运行状态: bash $0 status"
    print_info "查看日志: tail -f $PROJECT_ROOT/logs/*.log (如果有日志配置)"
}

# 停止所有服务器
stop_servers() {
    print_info "停止项目服务器..."
    
    local stopped_count=0
    
    # 先停止 Skynet
    stop_skynet
    
    # 查找并停止所有服务器进程
    for server in central db login gateway file matching; do
        if pgrep -x "$server" > /dev/null; then
            print_info "停止 $server 服务器..."
            pkill -TERM "$server"
            sleep 1
            
            # 检查是否还在运行
            if pgrep -x "$server" > /dev/null; then
                print_warning "$server 未响应，强制停止..."
                pkill -KILL "$server"
                sleep 0.5
            fi
            
            # 最终检查
            if ! pgrep -x "$server" > /dev/null; then
                print_success "$server 已停止"
                ((stopped_count++))
            else
                print_error "$server 停止失败"
            fi
        fi
    done
    
    echo ""
    if [ $stopped_count -eq 0 ]; then
        print_info "没有运行中的服务器"
    else
        print_success "已停止 $stopped_count 个服务器"
    fi
}

# 查看服务器状态
status_servers() {
    print_info "服务器运行状态:"
    echo ""
    
    # 显示项目路径
    print_info "项目根目录: $PROJECT_ROOT"
    print_info "构建目录:   $BUILD_DIR"
    echo ""
    
    # 检查 Redis
    echo -n "Redis:     "
    if redis-cli ping &>/dev/null; then
        echo -e "${GREEN}运行中${NC}"
    else
        echo -e "${RED}未运行${NC}"
    fi
    
    # 检查 MySQL
    if command -v mysql &>/dev/null; then
        echo -n "MySQL:     "
        if check_mysql_installed; then
            if sudo service mysql status &>/dev/null 2>&1; then
                echo -e "${GREEN}运行中${NC}"
            else
                echo -e "${RED}未运行${NC}"
            fi
        else
            echo -e "${YELLOW}未安装 (可能使用 Windows 的 MySQL)${NC}"
        fi
    fi
    
    echo ""
    
    # 检查 Skynet
    echo -n "$(printf '%-10s' "skynet"): "
    if pgrep -x "skynet" > /dev/null; then
        pid=$(pgrep -x "skynet")
        echo -e "${GREEN}运行中${NC} (PID: $pid)"
    else
        echo -e "${YELLOW}未运行${NC}"
    fi
    
    # 检查各个服务器
    local running_count=0
    for server in central db login gateway file matching; do
        echo -n "$(printf '%-10s' $server): "
        if pgrep -x "$server" > /dev/null; then
            pid=$(pgrep -x "$server")
            echo -e "${GREEN}运行中${NC} (PID: $pid)"
            ((running_count++))
        else
            echo -e "${YELLOW}未运行${NC}"
        fi
    done
    
    echo ""
    print_info "运行中的服务器: $running_count/6 + Skynet"
}

# 重启所有服务器
restart_servers() {
    print_info "重启项目服务器..."
    stop_servers
    sleep 2
    start_databases
    sleep 1
    start_servers
}

# 显示帮助信息
show_help() {
    echo "用法: bash $0 [命令]"
    echo ""
    echo "命令:"
    echo "  start-db    - 启动数据库服务（Redis 和 MySQL）"
    echo "  stop-db     - 停止数据库服务"
    echo "  start-skynet- 仅启动 Skynet 逻辑服务器"
    echo "  stop-skynet - 仅停止 Skynet 逻辑服务器"
    echo "  start       - 启动所有项目服务器（包括数据库和 Skynet）"
    echo "  stop        - 停止所有项目服务器"
    echo "  restart     - 重启所有项目服务器"
    echo "  status      - 查看服务器运行状态"
    echo "  copy-config - 仅复制配置文件"
    echo "  help        - 显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  bash $0 start-db      # 启动数据库"
    echo "  bash $0 start-skynet  # 仅启动 Skynet"
    echo "  bash $0 copy-config   # 复制配置文件"
    echo "  bash $0 start         # 启动所有服务器"
    echo "  bash $0 status        # 查看状态"
    echo ""
    echo "注意:"
    echo "  - 需要 Redis 服务才能启动项目服务器"
    echo "  - MySQL 服务是可选的（可使用 Windows 的 MySQL）"
    echo "  - Skynet 会在启动所有服务器时自动启动"
    echo "  - 首次启动或修改配置后，会自动复制配置文件"
    echo "  - 确保已编译项目: cd $PROJECT_ROOT && bash tools/debug/wsl/compile_guide.sh"
    echo ""
    echo "路径信息:"
    echo "  项目根目录: $PROJECT_ROOT"
    echo "  构建目录:   $BUILD_DIR"
    echo "  Skynet 目录: $SKYNET_DIR"
}

# 主函数
main() {
    case "${1:-help}" in
        start-db)
            start_databases
            ;;
        stop-db)
            stop_databases
            ;;
        start-skynet)
            start_skynet
            ;;
        stop-skynet)
            stop_skynet
            ;;
        copy-config)
            copy_configs
            ;;
        start)
            start_databases
            start_servers
            ;;
        stop)
            stop_servers
            ;;
        restart)
            restart_servers
            ;;
        status)
            status_servers
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            print_error "未知命令: $1"
            echo ""
            show_help
            exit 1
            ;;
    esac
}

main "$@"
