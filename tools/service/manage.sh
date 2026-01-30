#!/bin/bash
# 项目统一管理脚本（简化版）
# 调用其他独立脚本完成具体功能

set -e

# 加载公共函数
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

#==================== 帮助信息 ====================#

show_help() {
    cat << EOF
项目统一管理脚本

用法: bash $0 <命令> [参数]

快速命令:
  compile [Release|Debug]  - 完整编译流程（Proto + Build + Config）
  start                    - 启动所有服务
  stop                     - 停止所有服务
  restart                  - 重启所有服务
  status                   - 查看服务状态

日志命令:
  logs all [N]             - 查看所有日志最后 N 行
  logs f <服务>            - 实时追踪日志（gateway/skynet/...）
  logs fa                  - 实时追踪所有日志
  logs errors              - 查看错误日志
  logs search <关键词>     - 搜索日志

分步命令:
  proto                    - 生成 Protobuf 代码
  config                   - 复制配置文件
  build [Release|Debug]    - 编译项目（Skynet + C++）

示例:
  bash $0 compile          # 完整编译（Release 模式）
  bash $0 start            # 启动所有服务
  bash $0 status           # 查看状态
  bash $0 logs f gateway   # 实时追踪 gateway 日志
  bash $0 logs errors      # 查看错误日志

EOF
}

#==================== 主函数 ====================#

case "${1:-help}" in
    compile)
        # 完整编译流程
        print_header "完整编译流程"
        
        BUILD_TYPE="${2:-Release}"
        
        # 1. 生成 Proto
        bash "$SCRIPT_DIR/generate_proto.sh" || exit 1
        
        # 2. 编译项目
        bash "$SCRIPT_DIR/build.sh" "$BUILD_TYPE" || exit 1
        
        # 3. 复制配置
        bash "$SCRIPT_DIR/copy_config.sh" || exit 1
        
        print_header "编译完成"
        print_success "所有组件编译成功！"
        ;;
    
    proto)
        bash "$SCRIPT_DIR/generate_proto.sh"
        ;;
    
    config)
        bash "$SCRIPT_DIR/copy_config.sh"
        ;;
    
    build)
        bash "$SCRIPT_DIR/build.sh" "${2:-Release}"
        bash "$SCRIPT_DIR/copy_config.sh"
        ;;
    
    start)
        bash "$SCRIPT_DIR/service.sh" start
        ;;
    
    stop)
            bash "$SCRIPT_DIR/service.sh" stop
            ;;
    
        restart)
            bash "$SCRIPT_DIR/service.sh" restart
            ;;
    
        status)
            bash "$SCRIPT_DIR/service.sh" status
            ;;
    
        # 日志命令
        logs)
            bash "$SCRIPT_DIR/../logs.sh" "${@:2}"
            ;;
    
        log)
            bash "$SCRIPT_DIR/../logs.sh" "${@:2}"
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
