rem 用于debug时，正确处理配置文件。修改配置文件后，需要执行此脚本
rem 使用读取配置文件时，以执行文件为原点，路径为：/config/***.lua

rem 将配置文件复制到可执行文件同级文件夹中
rem 将 /config/server_config.lua 文件分割（每个服务器只保留相关配置）

rem 设置可执行文件路径
set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..\..
rem 顶级模块
set DEBUG_DIR=%PROJECT_DIR%\out\build\x64-Debug
rem 子模块
set CENTRAL_DIR=%DEBUG_DIR%\src\central
set DB_DIR=%DEBUG_DIR%\src\data
set GATEWAY_DIR=%DEBUG_DIR%\src\gateway
set LOGIC_DIR=%DEBUG_DIR%\src\logic
set LOGIN_DIR=%DEBUG_DIR%\src\login

rem 复制数据库配置文件
copy /y %PROJECT_DIR%\config\server_config.lua %DB_DIR%\config

rem 复制服务器配置文件
