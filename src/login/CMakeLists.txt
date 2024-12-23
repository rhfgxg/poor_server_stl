# login用户登录服务器模块，管理用户登录登出请求
# 模块名
set(MODULE_NAME login)

# 定义常量
set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")
set(VCPKG_INCLUDE "${VCPKG_DIR}/include")
set(VCPKG_LIB "${VCPKG_DIR}/lib")
set(VCPKG_BIN "${VCPKG_DIR}/bin")
set(VCPKG_DEBUG_LIB "${VCPKG_DIR}/debug/lib")
set(VCPKG_DEBUG_BIN "${VCPKG_DIR}/debug/bin")

# 包含自定义头文件的目录
include_directories(
    ${PROJECT_SOURCE_DIR}/src/login/src/login
    ${PROJECT_SOURCE_DIR}/src/common
    # 其他在顶级目录包含的头文件目录
)

# 需要编译的源文件列表
set(SOURCES
    # 源文件
    ${PROJECT_SOURCE_DIR}/src/login/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/login/src/login/login_server.cpp
    # 共享工具类
    ${PROJECT_SOURCE_DIR}/src/common/connection_pool.cpp
    ${PROJECT_SOURCE_DIR}/src/common/logger_manager.cpp
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

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4244 /utf-8")   # 忽略警告，使用utf-8编码
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /D_ITERATOR_DEBUG_LEVEL=2")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /D_ITERATOR_DEBUG_LEVEL=0")
add_compile_options(/wd4828)    # 忽略utf-8相关警告

# 如果需要链接第三方库，可以在这里指定（项目名，库文件权限，库名列表）
target_link_libraries(${MODULE_NAME} PRIVATE
    # boost 库
    debug "${VCPKG_DEBUG_LIB}/boost_system-vc143-mt-gd-x64-1_85.lib"
    optimized "${VCPKG_LIB}/boost_system-vc143-mt-x64-1_85.lib"
    # grpc 库
    debug "${VCPKG_DEBUG_LIB}/gpr.lib"
    optimized "${VCPKG_LIB}/gpr.lib"
    debug "${VCPKG_DEBUG_LIB}/grpc.lib"
    optimized "${VCPKG_LIB}/grpc.lib"
    debug "${VCPKG_DEBUG_LIB}/grpc_unsecure.lib"
    optimized "${VCPKG_LIB}/grpc_unsecure.lib"
    debug "${VCPKG_DEBUG_LIB}/grpc++.lib"
    optimized "${VCPKG_LIB}/grpc++.lib"
    debug "${VCPKG_DEBUG_LIB}/grpc++_unsecure.lib"
    optimized "${VCPKG_LIB}/grpc++_unsecure.lib"
    # protobuf 库
    debug "${VCPKG_DEBUG_LIB}/libprotobufd.lib"
    optimized "${VCPKG_LIB}/libprotobuf.lib"
    debug "${VCPKG_DEBUG_LIB}/libprotocd.lib"
    optimized "${VCPKG_LIB}/libprotoc.lib"
    # openssl 库
    debug "${VCPKG_DEBUG_LIB}/libssl.lib"
    optimized "${VCPKG_LIB}/libssl.lib"
    debug "${VCPKG_DEBUG_LIB}/libcrypto.lib"
    optimized "${VCPKG_LIB}/libcrypto.lib"
    # absl 库
    debug "${VCPKG_DEBUG_LIB}/abseil_dll.lib"
    optimized "${VCPKG_LIB}/abseil_dll.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_commandlineflag.lib"
    optimized "${VCPKG_LIB}/absl_flags_commandlineflag.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_commandlineflag_internal.lib"
    optimized "${VCPKG_LIB}/absl_flags_commandlineflag_internal.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_config.lib"
    optimized "${VCPKG_LIB}/absl_flags_config.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_internal.lib"
    optimized "${VCPKG_LIB}/absl_flags_internal.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_marshalling.lib"
    optimized "${VCPKG_LIB}/absl_flags_marshalling.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_parse.lib"
    optimized "${VCPKG_LIB}/absl_flags_parse.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_private_handle_accessor.lib"
    optimized "${VCPKG_LIB}/absl_flags_private_handle_accessor.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_program_name.lib"
    optimized "${VCPKG_LIB}/absl_flags_program_name.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_reflection.lib"
    optimized "${VCPKG_LIB}/absl_flags_reflection.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_usage.lib"
    optimized "${VCPKG_LIB}/absl_flags_usage.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_flags_usage_internal.lib"
    optimized "${VCPKG_LIB}/absl_flags_usage_internal.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_log_flags.lib"
    optimized "${VCPKG_LIB}/absl_log_flags.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_poison.lib"
    optimized "${VCPKG_LIB}/absl_poison.lib"
    debug "${VCPKG_DEBUG_LIB}/absl_utf8_for_code_point.lib"
    optimized "${VCPKG_LIB}/absl_utf8_for_code_point.lib"
    # zlib 库
    debug "${VCPKG_DEBUG_LIB}/zlibd.lib"
    optimized "${VCPKG_LIB}/zlib.lib"
    # re2 库
    debug "${VCPKG_DEBUG_LIB}/re2.lib"
    optimized "${VCPKG_LIB}/re2.lib"
    # upb 库
    debug "${VCPKG_DEBUG_LIB}/upb.lib"
    optimized "${VCPKG_LIB}/upb.lib"
    debug "${VCPKG_DEBUG_LIB}/utf8_range.lib"
    optimized "${VCPKG_LIB}/utf8_range.lib"
    debug "${VCPKG_DEBUG_LIB}/utf8_validity.lib"
    optimized "${VCPKG_LIB}/utf8_validity.lib"
    # c-ares 库
    debug "${VCPKG_DEBUG_LIB}/cares.lib"
    optimized "${VCPKG_LIB}/cares.lib"
    # address_sorting 库
    debug "${VCPKG_DEBUG_LIB}/address_sorting.lib"
    optimized "${VCPKG_LIB}/address_sorting.lib"
    # fmt 库
    debug "${VCPKG_DEBUG_LIB}/fmtd.lib"
    optimized "${VCPKG_LIB}/fmt.lib"
    # spdlog 库
    debug "${VCPKG_DEBUG_LIB}/spdlogd.lib"
    optimized "${VCPKG_LIB}/spdlog.lib"
)

# 创建一个列表，包含所有需要复制的 .dll 文件
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DLL_FILES
        "${VCPKG_DEBUG_BIN}/boost_system-vc143-mt-gd-x64-1_85.dll"
        "${VCPKG_DEBUG_BIN}/libprotobufd.dll"
        "${VCPKG_DEBUG_BIN}/libprotocd.dll"
        "${VCPKG_DEBUG_BIN}/libssl-3-x64.dll"
        "${VCPKG_DEBUG_BIN}/libcrypto-3-x64.dll"
        "${VCPKG_DEBUG_BIN}/abseil_dll.dll"
        "${VCPKG_DEBUG_BIN}/zlibd1.dll"
        "${VCPKG_DEBUG_BIN}/re2.dll"
        "${VCPKG_DEBUG_BIN}/cares.dll"
        "${VCPKG_DEBUG_BIN}/fmtd.dll"
        "${VCPKG_DEBUG_BIN}/spdlogd.dll"
    )
else()
    set(DLL_FILES
        "${VCPKG_BIN}/boost_system-vc143-mt-x64-1_85.dll"
        "${VCPKG_BIN}/libprotobuf.dll"
        "${VCPKG_BIN}/libprotoc.dll"
        "${VCPKG_BIN}/libssl-3-x64.dll"
        "${VCPKG_BIN}/libcrypto-3-x64.dll"
        "${VCPKG_BIN}/abseil_dll.dll"
        "${VCPKG_BIN}/zlib1.dll"
        "${VCPKG_BIN}/re2.dll"
        "${VCPKG_BIN}/cares.dll"
        "${VCPKG_BIN}/fmt.dll"
        "${VCPKG_BIN}/spdlog.dll"
    )
endif()

# 遍历 dll列表，将所有 .dll 文件，逐一复制到可执行文件所在目录
foreach(DLL_FILE ${DLL_FILES})
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_FILE}"
        $<TARGET_FILE_DIR:${MODULE_NAME}>  # 复制到可执行文件生成目录
    )
endforeach()

# 安装可执行文件到 bin 目录
install(TARGETS ${MODULE_NAME} DESTINATION bin)
