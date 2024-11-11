# 指定 CMake 的最低版本要求
cmake_minimum_required (VERSION 3.8)

# 设置项目名称(name)和版本(1.0)
project("poor_server_stl" VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)   # 强制要求指定的 C++ 标准必须被支持。如果编译器不支持这个标准，CMake 将会生成错误并停止构建

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# 定义常量
# 设置vcpkg路径
# set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")  # 替换为实际vcpkg路径
# set(VCPKG_INCLUDE "${VCPKG_DIR}/include")    # vcpkg的第三方库 include文件夹
# set(VCPKG_LIB "${VCPKG_DIR}/lib")    # vcpkg的第三方库 lib静态链接库文件夹
# set(VCPKG_BIN "${VCPKG_DIR}/bin")    # vcpkg的第三方库 dll动态链接库文件夹

# 包含自定义头文件的目录
include_directories(
   # ${PROJECT_SOURCE_DIR}/include
   ${VCPKG_INCLUDE}     # 指定第三方库的头文件所在目录
)

# 项目中所有自定义文件的 源文件（只需要cpp文件）的路径（.h或资源文件不需要添加）
# 包含的文件才会被编译
set(SOURCES
    src/main.cpp
)

# 引入Protobuf和RPC库
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# 添加每个模块
add_subdirectory(core)
add_subdirectory(data)
add_subdirectory(gateway)
add_subdirectory(logic)
add_subdirectory(login)


# 指定编译生成的可执行文件信息：文件名${PROJECT_NAME} ，${SOURCES} 项目所有源文件路径
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE core data gateway logic login)


# 安装可执行文件到 bin 目录
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
