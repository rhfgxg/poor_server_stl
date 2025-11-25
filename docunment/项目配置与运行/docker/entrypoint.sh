#!/bin/bash
# Docker 容器启动脚本

set -e

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

# 等待 Redis 启动
wait_for_redis() {
    print_info "等待 Redis 服务..."
    local max_attempts=30
    local attempt=1
    
    while [ $attempt -le $max_attempts ]; do
        if redis-cli -h ${REDIS_HOST:-localhost} -p ${REDIS_PORT:-6379} ping &>/dev/null; then
            print_success "Redis 已就绪"
            return 0
        fi
        print_info "等待 Redis... ($attempt/$max_attempts)"
        sleep 2
        ((attempt++))
    done
    
    print_error "Redis 连接超时"
    return 1
}

# 等待 MySQL 启动
wait_for_mysql() {
    print_info "等待 MySQL 服务..."
    local max_attempts=30
    local attempt=1
    
    while [ $attempt -le $max_attempts ]; do
        if mysqladmin ping -h ${MYSQL_HOST:-localhost} -P ${MYSQL_PORT:-3306} &>/dev/null; then
            print_success "MySQL 已就绪"
            return 0
        fi
        print_info "等待 MySQL... ($attempt/$max_attempts)"
        sleep 2
        ((attempt++))
    done
    
    print_warning "MySQL 连接超时（如果不需要 MySQL 可以忽略）"
    return 0
}

# 启动单个服务
start_service() {
    local service_name=$1
    local service_bin="/app/bin/${service_name}"
    
    if [ ! -f "$service_bin" ]; then
        print_warning "服务 ${service_name} 不存在"
        return 1
    fi
    
    print_info "启动 ${service_name} 服务..."
    
    # 在后台启动服务
    ${service_bin} &
    local pid=$!
    
    # 检查是否启动成功
    sleep 2
    if ps -p $pid > /dev/null; then
        print_success "${service_name} 已启动 (PID: $pid)"
        return 0
    else
        print_error "${service_name} 启动失败"
        return 1
    fi
}

# 主函数
main() {
    print_info "======================================="
    print_info "  Poor Server STL - Docker Container  "
    print_info "======================================="
    echo ""
    
    # 显示环境信息
    print_info "环境变量："
    print_info "  REDIS_HOST: ${REDIS_HOST:-localhost}"
    print_info "  REDIS_PORT: ${REDIS_PORT:-6379}"
    print_info "  MYSQL_HOST: ${MYSQL_HOST:-localhost}"
    print_info "  MYSQL_PORT: ${MYSQL_PORT:-3306}"
    echo ""
    
    # 等待依赖服务
    if [ "${WAIT_FOR_SERVICES}" = "true" ]; then
        wait_for_redis || exit 1
        wait_for_mysql
    fi
    
    # 根据参数启动服务
    case "${1:-all}" in
        all)
            print_info "启动所有服务..."
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
            ;;
        central)
            start_service "central"
            ;;
        db)
            start_service "db"
            ;;
        login)
            start_service "login"
            ;;
        gateway)
            start_service "gateway"
            ;;
        file)
            start_service "file"
            ;;
        matching)
            start_service "matching"
            ;;
        *)
            print_error "未知服务: $1"
            print_info "可用服务: all, central, db, login, gateway, file, matching"
            exit 1
            ;;
    esac
    
    echo ""
    print_success "======================================="
    print_success "  所有服务已启动！"
    print_success "======================================="
    echo ""
    print_info "使用 'docker logs -f <container>' 查看日志"
    
    # 保持容器运行
    # 监控所有子进程，如果任何一个退出则容器也退出
    wait
}

# 捕获退出信号
trap 'print_info "收到退出信号，正在关闭..."; killall central db login gateway file matching; exit 0' SIGTERM SIGINT

main "$@"
