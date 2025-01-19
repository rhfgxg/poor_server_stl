# login用户登录服务器模块，管理用户登录登出请求
# 模块名
set(MODULE_NAME login)

# 需要编译的源文件列表
set(SOURCES
    # 源文件
    ${PROJECT_SOURCE_DIR}/src/login/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/login/src/login/login_server.cpp
    # 共享工具类
    ${PROJECT_SOURCE_DIR}/src/common/connection_pool.cpp
    ${PROJECT_SOURCE_DIR}/src/common/logger_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/common/redis_client.cpp
    # protobuf 生成的源文件
    ${PROJECT_SOURCE_DIR}/protobuf/central/server_central.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/central/server_central.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/data/server_data.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/data/server_data.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/login/server_login.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/login/server_login.grpc.pb.cc
)

# 指定编译生成的可执行文件信息：文件名${MODULE_NAME} ，${SOURCES} 项目所有源文件路径
add_executable(${MODULE_NAME} ${SOURCES})

# 向其他模块提供接口，将一些公共头文件指定为 PUBLIC，其他模块可以访问
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 链接第三方库lib
target_link_libraries(${MODULE_NAME} PRIVATE
# 注释了一些暂时不需要的库
#    absl::base
#    absl::flags
#    Boost::system
#    c-ares::cares
#    fmt::fmt
#    OpenSSL::SSL
#    OpenSSL::Crypto
#    protobuf::libprotobuf
#    re2::re2
#    ZLIB::ZLIB
#    upb::upb
    gRPC::grpc++
    lua
    spdlog::spdlog
    ${ADDRESS_SORTING_LIB}  # address_sorting 库
    "${VCPKG_DEBUG_LIB}/cpp_redis.lib"    # cpp_redis
    "${VCPKG_DEBUG_LIB}/tacopie.lib"    # tacopie 库
)

# 复制其他dll文件到可执行文件所在目录
foreach(DLL_FILE ${DLL_FILES})
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_FILE}"
        $<TARGET_FILE_DIR:${MODULE_NAME}>  # 复制到可执行文件生成目录
    )
endforeach()

# 安装可执行文件到 bin 目录
install(TARGETS ${MODULE_NAME} DESTINATION bin)
