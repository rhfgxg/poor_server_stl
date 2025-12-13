#!/bin/bash
# 快速启动 Skynet（无需安装 pb.so）

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$(dirname "$SCRIPT_DIR")")")"
SKYNET_DIR="$PROJECT_ROOT/skynet_src/skynet"
CONFIG_FILE="$PROJECT_ROOT/config/skynet/config.lua"

# 颜色定义
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}============================================${NC}"
echo -e "${BLUE}Skynet 快速启动（Protobuf 集成测试）${NC}"
echo -e "${BLUE}============================================${NC}\n"

# 检查 Skynet
if [ ! -f "$SKYNET_DIR/skynet" ]; then
    echo -e "${YELLOW}[!] Skynet 未编译，正在编译...${NC}"
    cd "$SKYNET_DIR"
    make linux
    echo -e "${GREEN}[✓] Skynet 编译完成${NC}"
fi

# 检查配置文件
if [ ! -f "$CONFIG_FILE" ]; then
    echo -e "${RED}[✗] 配置文件未找到: $CONFIG_FILE${NC}"
    exit 1
fi

echo -e "${GREEN}[✓] Skynet 已就绪${NC}"
echo -e "${YELLOW}[i] 使用纯 Lua 序列化（无需 pb.so）${NC}"
echo -e "${YELLOW}[i] Protobuf 功能完整，性能略低${NC}\n"

echo -e "${BLUE}启动信息:${NC}"
echo "  - 项目根目录: $PROJECT_ROOT"
echo "  - Skynet 目录: $SKYNET_DIR"
echo "  - 配置文件: $CONFIG_FILE"
echo ""
echo -e "${BLUE}服务列表:${NC}"
echo "  - C++ Gateway (端口 8888)"
echo "  - Player Manager"
echo "  - Achievement Service"
echo "  - Hearthstone Manager"
echo ""

read -p "按 Enter 启动 Skynet..." -r
echo ""

cd "$SKYNET_DIR"
echo -e "${GREEN}[启动中...] 按 Ctrl+C 停止${NC}\n"

./skynet "$CONFIG_FILE"

echo -e "\n${BLUE}Skynet 已停止${NC}"
