# 定义常量
set(PROTOBUF_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 需要编译的源文件列表
set(PROTO_SOURCES
    ${PROTOBUF_DIR}/service.pb.cc
    ${PROTOBUF_DIR}/service.grpc.pb.cc
)

# 添加库
add_library(protobuf_files ${PROTO_SOURCES})

# 包含自定义头文件的目录
target_include_directories(protobuf_files PUBLIC ${PROTOBUF_DIR})

# 链接库
target_link_libraries(protobuf_files PUBLIC protobuf::libprotobuf grpc::grpc++)

# 向其他模块提供接口，将一些公共头文件指定为 PUBLIC，其他模块可以访问
target_include_directories(protobuf_files PUBLIC ${PROTOBUF_DIR})
