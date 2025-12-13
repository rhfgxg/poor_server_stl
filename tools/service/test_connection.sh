#!/bin/bash
# 测试 Skynet 连接
# 功能：检查 Skynet 是否运行并测试连接

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}============================================${NC}"
echo -e "${BLUE}测试 Skynet 连接${NC}"
echo -e "${BLUE}============================================${NC}\n"

echo -e "${YELLOW}[i] 检查 Skynet 是否运行...${NC}"

# 检查端口 8888
if lsof -i:8888 > /dev/null 2>&1 || netstat -tuln 2>/dev/null | grep -q ":8888 "; then
    echo -e "${GREEN}[✓] Skynet 正在运行（端口 8888 已监听）${NC}"
else
    echo -e "${RED}[✗] Skynet 未运行或端口 8888 未监听${NC}"
    echo ""
    echo "请先启动 Skynet:"
    echo "  bash $PROJECT_ROOT/tools/service/start_skynet.sh"
    exit 1
fi

echo ""
echo -e "${BLUE}测试连接:${NC}"

# 使用 nc 或 telnet 测试
if command -v nc &> /dev/null; then
    echo -e "${YELLOW}[i] 使用 nc 测试连接到 127.0.0.1:8888...${NC}"
    timeout 2 nc -zv 127.0.0.1 8888 && echo -e "${GREEN}[✓] 连接成功!${NC}" || echo -e "${RED}[✗] 连接失败${NC}"
elif command -v telnet &> /dev/null; then
    echo -e "${YELLOW}[i] 使用 telnet 测试连接到 127.0.0.1:8888...${NC}"
    timeout 2 telnet 127.0.0.1 8888 && echo -e "${GREEN}[✓] 连接成功!${NC}" || echo -e "${RED}[✗] 连接失败${NC}"
else
    echo -e "${YELLOW}[!] nc 和 telnet 都不可用，跳过连接测试${NC}"
fi

echo ""
echo -e "${BLUE}下一步:${NC}"
echo "  1. 在 C++ Gateway 中集成 SkynetClient"
echo "  2. 发送测试消息（EnterGameRequest）"
echo "  3. 验证响应"
echo ""
echo -e "${GREEN}测试完成!${NC}"
