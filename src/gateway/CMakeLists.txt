# gateway/CMakeLists.txt
# 模块名
set(MODULE_NAME gateway)

# 创建库
add_library(${MODULE_NAME} STATIC GatewayServer.cpp GatewayServer.h)

# 设置模块的包含目录
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 其他依赖，使用变量进行链接
target_link_libraries(${MODULE_NAME} PRIVATE protobuf)  # 假设需要Protobuf
