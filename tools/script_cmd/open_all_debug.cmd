@echo off
rem 启动所有服务器

rem 设置调试目录
set DEBUG_DIR=D:\Project\cpp\APP\poor\poor_server_stl\out\build\x64-Debug

rem 中心服务器
start "CentralServer" cmd /k "%DEBUG_DIR%\src\central\central.exe"
rem 数据库服务器
start "DataServer" cmd /k "%DEBUG_DIR%\src\data\data.exe"
rem 登录服务器
start "LoginServer" cmd /k "%DEBUG_DIR%\src\login\login.exe"
rem 逻辑服务器
rem start "LogicServer" cmd /k "%DEBUG_DIR%\src\logic\logic.exe"
rem 网关服务器
start "GatewayServer" cmd /k "%DEBUG_DIR%\src\gateway\gateway.exe"

pause