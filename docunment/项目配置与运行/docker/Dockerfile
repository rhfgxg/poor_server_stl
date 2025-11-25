# poor_server_stl 游戏服务器 Docker 镜像
# 基于 Ubuntu 22.04，包含所有运行时依赖

# ==================== 阶段 1: 构建阶段 ====================
FROM ubuntu:22.04 AS builder

# 设置环境变量
ENV DEBIAN_FRONTEND=noninteractive \
    TZ=Asia/Shanghai \
    LANG=C.UTF-8

# 安装构建依赖
RUN apt-get update && apt-get install -y \
    # 基础工具
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    curl \
    pkg-config \
    # 编译器
    gcc-11 \
    g++-11 \
    # Protocol Buffers 和 gRPC
    libprotobuf-dev \
    protobuf-compiler \
    libgrpc++-dev \
    # 数据库客户端库
    libmysqlclient-dev \
    # Redis
    libhiredis-dev \
    # 其他库
    libssl-dev \
    zlib1g-dev \
    liblua5.3-dev \
    # 清理缓存
    && rm -rf /var/lib/apt/lists/*

# 设置 GCC 11 为默认编译器
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100

# 安装 vcpkg
WORKDIR /opt
RUN git clone https://github.com/microsoft/vcpkg.git \
    && cd vcpkg \
    && ./bootstrap-vcpkg.sh \
    && ./vcpkg integrate install

# 设置工作目录
WORKDIR /build

# 复制项目文件
COPY . .

# 安装项目依赖（使用 vcpkg）
RUN /opt/vcpkg/vcpkg install \
    --triplet=x64-linux \
    --x-manifest-root=.

# 生成 Protocol Buffers 代码
RUN bash tools/debug/wsl/proto_make_cpp.sh && \
    bash tools/debug/wsl/proto_make_lua.sh

# 编译项目
RUN mkdir -p build && cd build && \
    cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    .. && \
    ninja

# 复制配置文件
RUN bash tools/debug/wsl/copy_config.sh

# ==================== 阶段 2: 运行阶段 ====================
FROM ubuntu:22.04 AS runtime

# 设置环境变量
ENV DEBIAN_FRONTEND=noninteractive \
    TZ=Asia/Shanghai \
    LANG=C.UTF-8

# 安装运行时依赖
RUN apt-get update && apt-get install -y \
    # C++ 运行时库
    libstdc++6 \
    # Protocol Buffers 运行时
    libprotobuf23 \
    # gRPC 运行时
    libgrpc++1.45 \
    # MySQL 客户端
    libmysqlclient21 \
    # Redis 客户端
    redis-server \
    # SSL 库
    libssl3 \
    # 其他运行时
    liblua5.3-0 \
    # 工具
    ca-certificates \
    # 清理缓存
    && rm -rf /var/lib/apt/lists/*

# 创建应用用户（安全最佳实践）
RUN groupadd -r gameserver && useradd -r -g gameserver gameserver

# 创建应用目录
RUN mkdir -p /app/bin /app/config /app/logs /app/data && \
    chown -R gameserver:gameserver /app

# 设置工作目录
WORKDIR /app

# 从构建阶段复制编译好的程序
COPY --from=builder --chown=gameserver:gameserver /build/build/src/central/central /app/bin/
COPY --from=builder --chown=gameserver:gameserver /build/build/src/db/db /app/bin/
COPY --from=builder --chown=gameserver:gameserver /build/build/src/gateway/gateway /app/bin/
COPY --from=builder --chown=gameserver:gameserver /build/build/src/login/login /app/bin/
COPY --from=builder --chown=gameserver:gameserver /build/build/src/file/file /app/bin/
COPY --from=builder --chown=gameserver:gameserver /build/build/src/matching/matching /app/bin/

# 复制配置文件
COPY --from=builder --chown=gameserver:gameserver /build/build/src/*/config /app/config/

# 复制脚本
COPY --chown=gameserver:gameserver docker/entrypoint.sh /app/
RUN chmod +x /app/entrypoint.sh

# 切换到应用用户
USER gameserver

# 暴露端口（根据实际配置调整）
EXPOSE 50051 50052 50053 50054 50055 50056

# 健康检查
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
    CMD pgrep -x central || exit 1

# 启动脚本
ENTRYPOINT ["/app/entrypoint.sh"]
CMD ["all"]
