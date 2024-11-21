# 定义常量
set(PROTOBUF_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 需要编译的源文件列表
set(PROTO_SOURCES
    ${PROTOBUF_DIR}/central/server_central.pb.cc
    ${PROTOBUF_DIR}/central/server_central.grpc.pb.cc
    ${PROTOBUF_DIR}/data/server_data.pb.cc
    ${PROTOBUF_DIR}/data/server_data.grpc.pb.cc
    ${PROTOBUF_DIR}/gateway/server_gateway.pb.cc
    ${PROTOBUF_DIR}/gateway/server_gateway.grpc.pb.cc
    ${PROTOBUF_DIR}/logic/server_logic.pb.cc
    ${PROTOBUF_DIR}/logic/server_logic.grpc.pb.cc
    ${PROTOBUF_DIR}/login/server_login.pb.cc
    ${PROTOBUF_DIR}/login/server_login.grpc.pb.cc
)

# 添加库
add_library(protobuf_files ${PROTO_SOURCES})

# 包含自定义头文件的目录
target_include_directories(protobuf_files PUBLIC ${PROTOBUF_DIR})


# 链接第三方库，可以在这里指定（项目名，库文件权限，库名列表）
target_link_libraries(protobuf_files PUBLIC
    # grpc 库
    "${VCPKG_LIB}/grpc++.lib"
    # protobuf 库
    "${VCPKG_LIB}/libprotobuf.lib"
)

# 向其他模块提供接口，将一些公共头文件指定为 PUBLIC，其他模块可以访问
target_include_directories(protobuf_files PUBLIC ${PROTOBUF_DIR})
