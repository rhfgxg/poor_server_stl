# data数据库服务器模块，管理所有数据库相关功能，其他模块通过此模块接口访问数据库
# 模块名
set(MODULE_NAME db)

# 需要编译的源文件列表
set(SOURCES
    # 源文件
    ${PROJECT_SOURCE_DIR}/src/db/main.cpp
    ${PROJECT_SOURCE_DIR}/src/db/server/db_server.cpp
    ${PROJECT_SOURCE_DIR}/src/db/server/db_connection_pool.cpp
    # 共享工具类
    ${PROJECT_SOURCE_DIR}/src/common/connection_pool.cpp
    ${PROJECT_SOURCE_DIR}/src/common/logger_manager.cpp
    # protobuf 生成的源文件
    ${PROJECT_SOURCE_DIR}/protobuf/src/common.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/src/common.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/src/server_central.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/src/server_central.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/src/server_db.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/src/server_db.grpc.pb.cc
)

# 指定编译生成的可执行文件信息：文件名${PROJECT_NAME} ，${SOURCES} 项目所有源文件路径
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
    spdlog::spdlog
    ${LUA_LIB}
    ${TACOPIE_LIB}
    ${CPPREDIS_LIB}
    ${MYSQLCPPCONN_LIB}  # mysql库
)


# 第三方库 dll列表
set(DLL_FILES
    "${VCPKG_DEBUG_BIN}/boost_system-vc143-mt-gd-x64-1_85.dll"
    "${VCPKG_DEBUG_BIN}/libprotobufd.dll"
    "${VCPKG_DEBUG_BIN}/libprotocd.dll"
    "${VCPKG_DEBUG_BIN}/libprotobuf-lited.dll"
    "${VCPKG_DEBUG_BIN}/libssl-3-x64.dll"
    "${VCPKG_DEBUG_BIN}/libcrypto-3-x64.dll"
    "${VCPKG_DEBUG_BIN}/abseil_dll.dll"
    "${VCPKG_BIN}/zlib1.dll"
    "${VCPKG_DEBUG_BIN}/zlibd1.dll"
    "${VCPKG_DEBUG_BIN}/zstd.dll"
    "${VCPKG_DEBUG_BIN}/re2.dll"
    "${VCPKG_DEBUG_BIN}/cares.dll"
    "${VCPKG_DEBUG_BIN}/fmtd.dll"
    "${VCPKG_DEBUG_BIN}/spdlogd.dll"
    "${VCPKG_DEBUG_BIN}/lua.dll"
    # mysql库
    "${VCPKG_DEBUG_BIN}/mysqlcppconn8-2-vs14.dll"
    "${VCPKG_DEBUG_BIN}/lz4d.dll"
)

# 复制dll文件到可执行文件所在目录
foreach(DLL_FILE ${DLL_FILES})
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_FILE}"
        $<TARGET_FILE_DIR:${MODULE_NAME}>  # 复制到可执行文件生成目录
    )
endforeach()


# 安装可执行文件到 bin 目录
install(TARGETS ${MODULE_NAME} DESTINATION bin)
