# login用户登录服务器模块，管理用户登录登出请求
# 模块名
set(MODULE_NAME login)

# 定义常量
set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")
set(VCPKG_INCLUDE "${VCPKG_DIR}/include")
set(VCPKG_LIB "${VCPKG_DIR}/lib")
set(VCPKG_BIN "${VCPKG_DIR}/bin")

# 包含自定义头文件的目录
include_directories(
   # ${PROJECT_SOURCE_DIR}/include
   ${VCPKG_INCLUDE}     # 指定第三方库的头文件所在目录
)

# 需要编译的源文件列表
set(SOURCES
    src/main.cpp
)

# 指定编译生成的可执行文件信息：文件名${PROJECT_NAME} ，${SOURCES} 项目所有源文件路径
add_executable(${MODULE_NAME} ${SOURCES})

# 设置模块的包含目录
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 向其他模块提供接口，将一些公共头文件指定为 PUBLIC，其他模块可以访问
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


# 如果需要链接第三方库，可以在这里指定（项目名，库文件权限，库名列表）
target_link_libraries(${MODULE_NAME} PRIVATE
# 第三方库
    "${VCPKG_LIB}/boost_system-vc143-mt-x64-1_85.lib" # boost 网络库
)


# 创建一个列表，包含所有需要复制的 .dll 文件
set(DLL_FILES
# boost 网络库
    "${VCPKG_BIN}/boost_system-vc143-mt-x64-1_85.dll"  
)

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