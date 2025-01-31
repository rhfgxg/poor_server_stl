# 定义常量
set(PROTOBUF_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4828 /utf-8")   # 忽略警告，使用utf-8编码
