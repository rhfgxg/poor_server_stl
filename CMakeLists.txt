cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 定义常量
project("poor_server_stl" VERSION 1.0)
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")
set(VCPKG_INCLUDE "${CMAKE_PREFIX_PATH}/include")
set(VCPKG_LIB "${CMAKE_PREFIX_PATH}/lib")
set(VCPKG_BIN "${CMAKE_PREFIX_PATH}/bin")
set(VCPKG_DEBUG_LIB "${CMAKE_PREFIX_PATH}/debug/lib")
set(VCPKG_DEBUG_BIN "${CMAKE_PREFIX_PATH}/debug/bin")
# 设置 vcpkg 工具链文件路径
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)    # 防止环境中定义了此变量
    set(CMAKE_TOOLCHAIN_FILE "D:/opt/tools/cpp/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()


# 包含头文件的目录，在子模块中生效
include_directories(
    ${VCPKG_INCLUDE}     # 第三方库的头文件所在目录
    # 共享工具类
    ${CMAKE_SOURCE_DIR}/src/common
    # 各服务器实现类
    ${CMAKE_SOURCE_DIR}/src/central
    ${CMAKE_SOURCE_DIR}/src/data
    ${CMAKE_SOURCE_DIR}/src/file
    ${CMAKE_SOURCE_DIR}/src/gateway
    ${CMAKE_SOURCE_DIR}/src/logic
    ${CMAKE_SOURCE_DIR}/src/login
    # proto生成的 grpc服务头文件目录
    ${CMAKE_SOURCE_DIR}/protobuf/central
    ${CMAKE_SOURCE_DIR}/protobuf/data
    ${CMAKE_SOURCE_DIR}/protobuf/file
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

# 自动寻找第三方库 lib列表
find_package(absl REQUIRED)
find_package(Boost REQUIRED system)
find_package(c-ares REQUIRED)
find_package(fmt REQUIRED)
find_package(gRPC REQUIRED)
find_package(Lua REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(re2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZLIB REQUIRED)
find_package(upb REQUIRED)

# 下面的库无法使用 find_package 寻找，需要手动指定
find_library(ADDRESS_SORTING_LIB address_sorting PATHS "${VCPKG_LIB}")  # address_sorting.lib, 用于 grpc
find_library(MYSQLCPPCONN_LIB mysqlcppconn8 PATHS "${VCPKG_LIB}")   # mysqlcppconn8.lib
# 下面文件需要直接在子模块中使用路径引入
# "${VCPKG_DEBUG_LIB}/cpp_redis.lib"    # cpp_redis
# "${VCPKG_DEBUG_LIB}/tacopie.lib"    # tacopie 库

# 添加每个模块，确保可以将所有变量
add_subdirectory(protobuf)
add_subdirectory(./src/central)
add_subdirectory(./src/data)
add_subdirectory(./src/file)
add_subdirectory(./src/gateway)
add_subdirectory(./src/logic)
add_subdirectory(./src/login)

# 安装可执行文件到 install/x64-Debug/bin 目录
install(TARGETS ${PROJECT_NAME} DESTINATION bin)