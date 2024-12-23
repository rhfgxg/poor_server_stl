cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 定义常量
project("poor_server_stl" VERSION 1.0)
set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")
set(VCPKG_INCLUDE "${VCPKG_DIR}/include")
set(VCPKG_LIB "${VCPKG_DIR}/lib")
set(VCPKG_BIN "${VCPKG_DIR}/bin")

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
    # proto生成的共享头文件目录
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

# 添加每个模块
add_subdirectory(protobuf)
add_subdirectory(./src/central)
add_subdirectory(./src/data)
add_subdirectory(./src/gateway)
add_subdirectory(./src/logic)
add_subdirectory(./src/login)

# 指定编译生成的可执行文件信息：文件名${PROJECT_NAME} ，${SOURCES} 项目所有源文件路径
add_executable(${PROJECT_NAME} ${SOURCES})

# 设置编译选项
# /utf-8：使用 utf-8 编码
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
add_compile_options(/wd4828)    # 忽略utf-8相关警告

# 如果需要链接第三方库，可以在这里指定（项目名，库文件权限，库名列表）
target_link_libraries(${PROJECT_NAME} PRIVATE
# 第三方库
    "${VCPKG_LIB}/libprotobufd.lib" # proto库
    "${VCPKG_LIB}/libprotocd.lib" # proto库
    # nlohmann：json库是一个头文件库，不存在lib文件，不需要链接
)


# 创建一个列表，包含所有需要复制的 .dll 文件
set(DLL_FILES
    "${VCPKG_BIN}/libprotobufd.dll"  # proto库
    "${VCPKG_BIN}/libprotocd.dll"          # proto库
)

# 遍历 dll列表，将所有 .dll 文件，逐一复制到可执行文件所在目录
foreach(DLL_FILE ${DLL_FILES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_FILE}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>  # 复制到可执行文件生成目录
    )
endforeach()


# 安装可执行文件到 bin 目录
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
