cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 定义常量
project("poor_server_stl" VERSION 1.0)
set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")
set(VCPKG_INCLUDE "${VCPKG_DIR}/include")
set(VCPKG_LIB "${VCPKG_DIR}/lib")
set(VCPKG_BIN "${VCPKG_DIR}/bin")
set(VCPKG_DEBUG_LIB "${VCPKG_DIR}/debug/lib")
set(VCPKG_DEBUG_BIN "${VCPKG_DIR}/debug/bin")

# 包含头文件的目录，在子模块中生效
include_directories(
    ${VCPKG_INCLUDE}     # 第三方库的头文件所在目录
    # 共享工具类
    ${CMAKE_SOURCE_DIR}/src/common
    # 各服务器实现类
    ${CMAKE_SOURCE_DIR}/src/central
    ${CMAKE_SOURCE_DIR}/src/data
    ${CMAKE_SOURCE_DIR}/src/gateway
    ${CMAKE_SOURCE_DIR}/src/logic
    ${CMAKE_SOURCE_DIR}/src/login
    # proto生成的 grpc服务头文件目录
    ${CMAKE_SOURCE_DIR}/protobuf/common
    ${CMAKE_SOURCE_DIR}/protobuf/central
    ${CMAKE_SOURCE_DIR}/protobuf/data
    ${CMAKE_SOURCE_DIR}/protobuf/gateway
    ${CMAKE_SOURCE_DIR}/protobuf/logic
    ${CMAKE_SOURCE_DIR}/protobuf/login
)

# 需要编译的源文件列表
set(SOURCES
    src/main.cpp
)

# 指定编译生成的可执行文件信息：文件名${PROJECT_NAME} ，${SOURCES} 项目所有源文件路径
add_executable(${PROJECT_NAME} ${SOURCES})

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4251 /wd4828 /utf-8")   # 忽略警告，使用utf-8编码
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /D_ITERATOR_DEBUG_LEVEL=2")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /D_ITERATOR_DEBUG_LEVEL=0")

# 第三方库 lib列表
set(THIRD_PARTY_LIBS
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
    # cpp_redis 和 tacopie 库
    debug "${VCPKG_DEBUG_LIB}/cpp_redis.lib"
    optimized "${VCPKG_LIB}/cpp_redis.lib"
    debug "${VCPKG_DEBUG_LIB}/tacopie.lib"
    optimized "${VCPKG_LIB}/tacopie.lib"
    # fmt 库
    debug "${VCPKG_DEBUG_LIB}/fmtd.lib"
    optimized "${VCPKG_LIB}/fmt.lib"
    # spdlog 库
    debug "${VCPKG_DEBUG_LIB}/spdlogd.lib"
    optimized "${VCPKG_LIB}/spdlog.lib"
    # lua 库
    debug "${VCPKG_DEBUG_LIB}/lua.lib"
    optimized "${VCPKG_LIB}/lua.lib"
)

# 第三方库 dll列表
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
        "${VCPKG_DEBUG_BIN}/lua.dll"
    )
else()
    set(DLL_FILES
        "${VCPKG_BIN}/boost_system-vc143-mt-x64-1_85.dll"   # boost 库
        # protobuf 库
        "${VCPKG_BIN}/libprotobuf.dll"
        "${VCPKG_BIN}/libprotoc.dll"    
        # openssl 库
        "${VCPKG_BIN}/libssl-3-x64.dll"
        "${VCPKG_BIN}/libcrypto-3-x64.dll"
        "${VCPKG_BIN}/abseil_dll.dll"   # abseil 库
        "${VCPKG_BIN}/zlib1.dll"    # zlib 库
        "${VCPKG_BIN}/re2.dll"  # re2 库
        "${VCPKG_BIN}/cares.dll"    # c-ares 库
        "${VCPKG_BIN}/fmt.dll"  # fmt 库
        "${VCPKG_BIN}/spdlog.dll"   # spdlog 库
        "${VCPKG_BIN}/lua.dll"  # lua 库
    )
endif()

# 添加每个模块，确保可以将所有变量
add_subdirectory(protobuf)
add_subdirectory(./src/central)
add_subdirectory(./src/data)
add_subdirectory(./src/gateway)
add_subdirectory(./src/logic)
add_subdirectory(./src/login)

# 链接第三方库
target_link_libraries(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_LIBS})

# 安装可执行文件到 install/x64-Debug/bin 目录
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# 遍历 dll列表，将所有 .dll 文件，逐一复制到可执行文件生成目录
foreach(DLL_FILE ${DLL_FILES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_FILE}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>  # 复制到可执行文件生成目录
    )
endforeach()

# 复制 Lua 配置文件到可执行文件生成目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/config"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config"
)