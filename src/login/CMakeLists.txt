# login用户登录服务器模块，管理用户登录登出请求
# 模块名
set(MODULE_NAME login)

# 需要编译的源文件列表
set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/login/main.cpp
    ${PROJECT_SOURCE_DIR}/src/login/server/login_server.cpp
    # 共享工具类
    ${PROJECT_SOURCE_DIR}/src/common/connection_pool.cpp
    ${PROJECT_SOURCE_DIR}/src/common/logger_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/common/redis_client.cpp
    ${PROJECT_SOURCE_DIR}/src/common/base_server.cpp
    ${PROJECT_SOURCE_DIR}/src/common/thread_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/common/config_manager.cpp
    # protobuf 生成的源文件
    ${PROJECT_SOURCE_DIR}/protobuf/cpp/src/common.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/cpp/src/common.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/cpp/src/server_central.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/cpp/src/server_central.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/cpp/src/server_db.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/cpp/src/server_db.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/cpp/src/server_login.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/cpp/src/server_login.grpc.pb.cc
)

# 指定编译生成的可执行文件信息
add_executable(${MODULE_NAME} ${SOURCES})

# 向其他模块提供接口
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 链接第三方库
target_link_libraries(${MODULE_NAME} PRIVATE
    gRPC::grpc++
    spdlog::spdlog
    ${LUA_LIB}
    ${CPPREDIS_LIB}
    ${TACOPIE_LIB}
)

# 第三方库 dll列表
if(WIN32)
    set(DLL_FILES
        "${VCPKG_DEBUG_BIN}/boost_system-vc143-mt-gd-x64-1_85.dll"
        "${VCPKG_DEBUG_BIN}/libprotobufd.dll"
        "${VCPKG_DEBUG_BIN}/libprotocd.dll"
        "${VCPKG_DEBUG_BIN}/libssl-3-x64.dll"
        "${VCPKG_DEBUG_BIN}/libcrypto-3-x64.dll"
        "${VCPKG_DEBUG_BIN}/abseil_dll.dll"
        "${VCPKG_BIN}/zlib1.dll"
        "${VCPKG_DEBUG_BIN}/re2.dll"
        "${VCPKG_DEBUG_BIN}/cares.dll"
        "${VCPKG_DEBUG_BIN}/fmtd.dll"
        "${VCPKG_DEBUG_BIN}/spdlogd.dll"
        "${VCPKG_DEBUG_BIN}/lua.dll"
    )
    foreach(DLL_FILE ${DLL_FILES})
        add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL_FILE}"
            $<TARGET_FILE_DIR:${MODULE_NAME}>
        )
    endforeach()
endif()

install(TARGETS ${MODULE_NAME} DESTINATION bin)
