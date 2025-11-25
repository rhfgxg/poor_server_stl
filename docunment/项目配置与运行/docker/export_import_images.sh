#!/bin/bash
# Docker 镜像导出/导入脚本
# 用于在无网络环境下分发 Docker 镜像

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

# 显示帮助信息
show_help() {
    cat << EOF
Docker 镜像导出/导入工具

用法: 
  bash $0 export [镜像名] [输出文件]    - 导出镜像到文件
  bash $0 import [镜像文件]            - 从文件导入镜像
  bash $0 export-all [输出目录]        - 导出所有项目镜像
  bash $0 import-all [输入目录]        - 导入目录中的所有镜像

示例:
  # 导出单个镜像
  bash $0 export poor-server:latest poor-server.tar
  
  # 导入单个镜像
  bash $0 import poor-server.tar
  
  # 导出所有项目镜像
  bash $0 export-all ./docker-images
  
  # 导入所有镜像
  bash $0 import-all ./docker-images

用途:
  1. 在无网络环境下分发镜像
  2. 备份镜像
  3. 团队成员之间共享开发环境
EOF
}

# 导出单个镜像
export_image() {
    local image=$1
    local output=$2
    
    if [ -z "$image" ] || [ -z "$output" ]; then
        print_error "用法: $0 export <镜像名> <输出文件>"
        exit 1
    fi
    
    # 检查镜像是否存在
    if ! docker image inspect "$image" &>/dev/null; then
        print_error "镜像不存在: $image"
        print_info "可用镜像列表:"
        docker images
        exit 1
    fi
    
    print_info "正在导出镜像: $image"
    print_info "输出文件: $output"
    
    # 导出镜像
    if docker save "$image" -o "$output"; then
        local size=$(du -h "$output" | cut -f1)
        print_success "镜像导出成功！"
        print_info "文件大小: $size"
        print_info "文件位置: $output"
        echo ""
        print_info "分享给团队成员的步骤:"
        print_info "1. 将文件 $output 发送给团队成员"
        print_info "2. 团队成员运行: docker load -i $output"
    else
        print_error "镜像导出失败"
        exit 1
    fi
}

# 导入单个镜像
import_image() {
    local input=$1
    
    if [ -z "$input" ]; then
        print_error "用法: $0 import <镜像文件>"
        exit 1
    fi
    
    # 检查文件是否存在
    if [ ! -f "$input" ]; then
        print_error "文件不存在: $input"
        exit 1
    fi
    
    print_info "正在导入镜像..."
    print_info "文件: $input"
    
    # 导入镜像
    if docker load -i "$input"; then
        print_success "镜像导入成功！"
        echo ""
        print_info "导入的镜像："
        docker images --format "table {{.Repository}}\t{{.Tag}}\t{{.Size}}" | head -5
    else
        print_error "镜像导入失败"
        exit 1
    fi
}

# 导出所有项目镜像
export_all() {
    local output_dir=${1:-./docker-images}
    
    print_info "导出所有项目相关镜像到: $output_dir"
    
    # 创建输出目录
    mkdir -p "$output_dir"
    
    # 项目镜像列表
    local images=(
        "poor-server:latest"
        "mysql:8.0"
        "redis:7-alpine"
    )
    
    local count=0
    
    for image in "${images[@]}"; do
        # 检查镜像是否存在
        if docker image inspect "$image" &>/dev/null; then
            local safe_name=$(echo "$image" | tr ':/' '_')
            local output_file="$output_dir/${safe_name}.tar"
            
            print_info "[$((++count))/${#images[@]}] 导出: $image"
            
            if docker save "$image" -o "$output_file"; then
                local size=$(du -h "$output_file" | cut -f1)
                print_success "  ? 导出成功 (大小: $size)"
            else
                print_error "  ? 导出失败"
            fi
        else
            print_warning "  ? 镜像不存在，跳过: $image"
        fi
    done
    
    echo ""
    print_success "导出完成！"
    print_info "输出目录: $output_dir"
    
    # 显示目录内容
    print_info "导出的文件："
    ls -lh "$output_dir"
    
    # 创建说明文件
    cat > "$output_dir/README.txt" << EOF
Poor Server STL - Docker 镜像包

导出时间: $(date)
导出机器: $(hostname)

包含镜像:
$(for img in "${images[@]}"; do echo "  - $img"; done)

导入方法:
==========
1. 将整个文件夹复制到目标机器
2. 进入该目录
3. 运行导入脚本:
   bash ../import_images.sh import-all .

或手动导入每个镜像:
   docker load -i poor-server_latest.tar
   docker load -i mysql_8.0.tar
   docker load -i redis_7-alpine.tar

导入后验证:
   docker images

启动项目:
   cd <项目目录>
   docker compose up -d
EOF
    
    print_info "已创建说明文件: $output_dir/README.txt"
    
    echo ""
    print_info "================================================================"
    print_info "下一步: 将文件夹 '$output_dir' 分享给团队成员"
    print_info "团队成员运行: bash $0 import-all $output_dir"
    print_info "================================================================"
}

# 导入所有镜像
import_all() {
    local input_dir=${1:-./docker-images}
    
    if [ ! -d "$input_dir" ]; then
        print_error "目录不存在: $input_dir"
        exit 1
    fi
    
    print_info "从目录导入所有镜像: $input_dir"
    echo ""
    
    # 查找所有 .tar 文件
    local tar_files=("$input_dir"/*.tar)
    
    if [ ${#tar_files[@]} -eq 0 ] || [ ! -f "${tar_files[0]}" ]; then
        print_error "目录中没有找到 .tar 文件"
        exit 1
    fi
    
    local count=0
    local success=0
    
    for tar_file in "${tar_files[@]}"; do
        ((count++))
        local filename=$(basename "$tar_file")
        
        print_info "[$count/${#tar_files[@]}] 导入: $filename"
        
        if docker load -i "$tar_file"; then
            ((success++))
            print_success "  ? 导入成功"
        else
            print_error "  ? 导入失败"
        fi
        echo ""
    done
    
    print_success "================================================================"
    print_success "导入完成！成功: $success/${#tar_files[@]}"
    print_success "================================================================"
    echo ""
    
    print_info "导入的镜像列表："
    docker images
    
    echo ""
    print_info "下一步:"
    print_info "  cd <项目目录>"
    print_info "  docker compose up -d"
}

# 主函数
main() {
    case "${1:-help}" in
        export)
            shift
            export_image "$@"
            ;;
        import)
            shift
            import_image "$@"
            ;;
        export-all)
            shift
            export_all "$@"
            ;;
        import-all)
            shift
            import_all "$@"
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
