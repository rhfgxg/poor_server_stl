@echo off
chcp 65001 >nul
rem 用于debug时，同步生成文件文件夹的配置文件。修改配置文件后，需要执行此脚本
rem 配置文件复制到 可执行文件同级的 /config 中

set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..\..
rem 配置文件目录
set CONFIG_DIR=%PROJECT_DIR%\config\cpp\cfg_server
rem 生成文件目录
set DEBUG_OUT_DIR=%PROJECT_DIR%\out\build\x64-debug

rem 子模块配置文件目录
set BATTLE_DIR=%DEBUG_OUT_DIR%\src\battle\config
set CENTRAL_DIR=%DEBUG_OUT_DIR%\src\central\config
set DB_DIR=%DEBUG_OUT_DIR%\src\db\config
set FILE_DIR=%DEBUG_OUT_DIR%\src\file\config
set GATEWAY_DIR=%DEBUG_OUT_DIR%\src\gateway\config
set LOGIC_DIR=%DEBUG_OUT_DIR%\src\logic\config
set LOGIN_DIR=%DEBUG_OUT_DIR%\src\login\config
set MATCHING_DIR=%DEBUG_OUT_DIR%\src\matching\config

rem 创建文件夹
if not exist %BATTLE_DIR% mkdir %BATTLE_DIR%
if not exist %CENTRAL_DIR% mkdir %CENTRAL_DIR%
if not exist %DB_DIR% mkdir %DB_DIR%
if not exist %FILE_DIR% mkdir %FILE_DIR%
if not exist %GATEWAY_DIR% mkdir %GATEWAY_DIR%
if not exist %LOGIC_DIR% mkdir %LOGIC_DIR%
if not exist %LOGIN_DIR% mkdir %LOGIN_DIR%
if not exist %MATCHING_DIR% mkdir %MATCHING_DIR%

rem 复制服务器配置文件
copy /y %CONFIG_DIR%\cfg_battle_server.lua	%BATTLE_DIR%
copy /y %CONFIG_DIR%\cfg_central_server.lua	%CENTRAL_DIR%
copy /y %CONFIG_DIR%\cfg_db_server.lua		%DB_DIR%
copy /y %CONFIG_DIR%\cfg_file_server.lua	%FILE_DIR%
copy /y %CONFIG_DIR%\cfg_gateway_server.lua %GATEWAY_DIR%
copy /y %CONFIG_DIR%\cfg_logic_server.lua	%LOGIC_DIR%
copy /y %CONFIG_DIR%\cfg_login_server.lua	%LOGIN_DIR%
copy /y %CONFIG_DIR%\cfg_matching_server.lua %MATCHING_DIR%

rem 复制数据库配置文件
copy /y %CONFIG_DIR%\cfg_db.lua %DB_DIR%

echo 脚本执行完毕
pause