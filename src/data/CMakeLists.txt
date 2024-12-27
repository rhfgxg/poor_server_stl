# data数据库服务器模块，管理所有数据库相关功能，其他模块通过此模块接口访问数据库
# 模块名
set(MODULE_NAME data)

# 需要编译的源文件列表
set(SOURCES
    # 源文件
    ${PROJECT_SOURCE_DIR}/src/data/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/data/src/data/data_server.cpp
    ${PROJECT_SOURCE_DIR}/src/data/src/data/de_connection_pool.cpp
    # 共享工具类
    ${PROJECT_SOURCE_DIR}/src/common/connection_pool.cpp
    ${PROJECT_SOURCE_DIR}/src/common/logger_manager.cpp
    # protobuf 生成的源文件
    ${PROJECT_SOURCE_DIR}/protobuf/central/server_central.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/central/server_central.grpc.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/data/server_data.pb.cc
    ${PROJECT_SOURCE_DIR}/protobuf/data/server_data.grpc.pb.cc
)

# 指定编译生成的可执行文件信息：文件名${PROJECT_NAME} ，${SOURCES} 项目所有源文件路径
add_executable(${MODULE_NAME} ${SOURCES})

# 向其他模块提供接口，将一些公共头文件指定为 PUBLIC，其他模块可以访问
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 链接第三方库lib
target_link_libraries(${MODULE_NAME} PRIVATE 
    ${THIRD_PARTY_LIBS}
    # mysql库
    debug "${VCPKG_DEBUG_LIB}/mysqlcppconn8.lib"
    optimized "${VCPKG_LIB}/mysqlcppconn8.lib"
)

# 第三方库 dll列表
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DLL_FILES
        # mysql库
        "${VCPKG_DEBUG_BIN}/mysqlcppconn8-2-vs14.dll"
        "${VCPKG_DEBUG_BIN}/lz4d.dll"
    )
else()
    set(DLL_FILES
        # mysql库
        "${VCPKG_DIR}/debug/bin/mysqlcppconn8-2-vs14.dll"  
        "${VCPKG_BIN}/lz4.dll"  # lz4 库
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
