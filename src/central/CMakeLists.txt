# central中心服务器模块，管理所有模块的交互，RPC调用
# 模块名
set(MODULE_NAME central)

# 需要编译的源文件列表
set(SOURCES
    # 源文件
    ${PROJECT_SOURCE_DIR}/src/central/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/central/src/central/central_server.cpp
    # 共享工具类
    ${PROJECT_SOURCE_DIR}/src/common/connection_pool.cpp
    ${PROJECT_SOURCE_DIR}/src/common/logger_manager.cpp
    # protobuf 生成的源文件：调用其他服务器
    ${PROJECT_SOURCE_DIR}/protobuf/central/server_central.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/central/server_central.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/data/server_data.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/data/server_data.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/gateway/server_gateway.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/gateway/server_gateway.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/logic/server_logic.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/logic/server_logic.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/login/server_login.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/login/server_login.grpc.pb.cc
)

# 指定编译生成的可执行文件信息：文件名${MODULE_NAME} ，${SOURCES} 项目所有源文件路径
add_executable(${MODULE_NAME} ${SOURCES})

# 向其他模块提供接口，将一些公共头文件指定为 PUBLIC，其他模块可以访问
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 链接第三方库lib
target_link_libraries(${MODULE_NAME} PRIVATE ${THIRD_PARTY_LIBS})

# 安装可执行文件到 bin 目录
install(TARGETS ${MODULE_NAME} DESTINATION bin)