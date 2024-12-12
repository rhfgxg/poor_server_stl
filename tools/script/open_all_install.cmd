@echo off
rem 启动所有服务器

rem 设置调试目录
set DEBUG_DIR=D:\Project\cpp\APP\poor\poor_server_stl\out\install\x64-Debug

rem 中心服务器
start "CentralServer" cmd /k "%DEBUG_DIR%\bin\central.exe"
rem 数据库服务器
start "DataServer" cmd /k "%DEBUG_DIR%\bin\data.exe"
rem 登录服务器
start "LoginServer" cmd /k "%DEBUG_DIR%\bin\login.exe"
rem 逻辑服务器
rem start "LogicServer" cmd /k "%DEBUG_DIR%\bin\logic.exe"
rem 网关服务器
start "GatewayServer" cmd /k "%DEBUG_DIR%\bin\gateway.exe"

pause