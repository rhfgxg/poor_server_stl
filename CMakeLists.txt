cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project("poor_server_stl" VERSION 1.0)

# 平台相关的 vcpkg 路径设置
if(WIN32)
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")
else()
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        if(DEFINED ENV{VCPKG_ROOT})
            set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
            set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-linux")
        endif()
    endif()
    # 同时设置 build 目录下的 vcpkg_installed 路径
    if(EXISTS "${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-linux")
        set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-linux" ${CMAKE_PREFIX_PATH})
    endif()
    if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux")
        set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux" ${CMAKE_PREFIX_PATH})
    endif()
endif()

# vcpkg 相关路径变量
set(VCPKG_INCLUDE "${CMAKE_PREFIX_PATH}/include")
set(VCPKG_LIB "${CMAKE_PREFIX_PATH}/lib")
set(VCPKG_BIN "${CMAKE_PREFIX_PATH}/bin")
set(VCPKG_DEBUG_LIB "${CMAKE_PREFIX_PATH}/debug/lib")
set(VCPKG_DEBUG_BIN "${CMAKE_PREFIX_PATH}/debug/bin")

# 头文件目录
include_directories(
    ${VCPKG_INCLUDE}
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/central
    ${CMAKE_SOURCE_DIR}/src/chat
    ${CMAKE_SOURCE_DIR}/src/db
    ${CMAKE_SOURCE_DIR}/src/file
    ${CMAKE_SOURCE_DIR}/src/gateway
    ${CMAKE_SOURCE_DIR}/src/login
    ${CMAKE_SOURCE_DIR}/src/matching
    ${CMAKE_SOURCE_DIR}/src/store
    ${CMAKE_SOURCE_DIR}/protobuf/cpp/src
)

# 编译选项
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4251 /wd4828 /utf-8")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC")
endif()

# 自动查找第三方库
find_package(absl REQUIRED)
find_package(Boost REQUIRED system)
find_package(c-ares REQUIRED)
find_package(fmt REQUIRED)
find_package(gRPC REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(re2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZLIB REQUIRED)
find_package(upb REQUIRED)

# 仅 find_package 找不到的库用 find_library
if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")   # 根据调试模式选择对应版本
        find_library(CPPREDIS_LIB cpp_redis PATHS "${VCPKG_DEBUG_LIB}" NO_DEFAULT_PATH)
        find_library(LUA_LIB lua PATHS "${VCPKG_DEBUG_LIB}")
        find_library(MYSQLCPPCONN_LIB mysqlcppconn8 PATHS "${VCPKG_DEBUG_LIB}")
        find_library(TACOPIE_LIB tacopie PATHS "${VCPKG_DEBUG_LIB}" NO_DEFAULT_PATH)
    else()
        find_library(CPPREDIS_LIB cpp_redis PATHS "${VCPKG_LIB}")
        find_library(LUA_LIB lua PATHS "${VCPKG_LIB}")
        find_library(MYSQLCPPCONN_LIB mysqlcppconn8 PATHS "${VCPKG_LIB}")
        find_library(TACOPIE_LIB tacopie PATHS "${VCPKG_LIB}")
    endif()
else()
    # Linux 环境下的库查找
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")   # 根据调试模式选择对应版本
        find_library(CPPREDIS_LIB cpp_redis PATHS "${VCPKG_DEBUG_LIB}")
        find_library(LUA_LIB lua PATHS "${VCPKG_DEBUG_LIB}")
        # 修复：Linux 下 MySQL 库名为 mysqlcppconn8-static
        find_library(MYSQLCPPCONN_LIB mysqlcppconn8-static 
            PATHS "${VCPKG_DEBUG_LIB}" "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/debug/lib"
            NO_DEFAULT_PATH)
        find_library(TACOPIE_LIB tacopie PATHS "${VCPKG_DEBUG_LIB}")
    else()
        find_library(CPPREDIS_LIB cpp_redis PATHS "${VCPKG_LIB}")
        find_library(LUA_LIB lua PATHS "${VCPKG_LIB}")
        # 修复：Linux 下 MySQL 库名为 mysqlcppconn8-static
        find_library(MYSQLCPPCONN_LIB mysqlcppconn8-static 
            PATHS "${VCPKG_LIB}" "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-linux/lib"
            NO_DEFAULT_PATH)
        find_library(TACOPIE_LIB tacopie PATHS "${VCPKG_LIB}")
    endif()
endif()

message(STATUS "MYSQLCPPCONN_LIB: ${MYSQLCPPCONN_LIB}")

# 添加子模块
add_subdirectory(protobuf/cpp)
add_subdirectory(src/central)
add_subdirectory(src/db)
add_subdirectory(src/file)
add_subdirectory(src/gateway)
add_subdirectory(src/login)
add_subdirectory(src/matching)