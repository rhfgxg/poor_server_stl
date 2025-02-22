@echo off
chcp 65001 >nul
rem 根据 proto文件，生成对应源码

rem 设置路径常量
set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..\..
set PROTO_DIR=%PROJECT_DIR%\protobuf
set MAKE_OUT=%PROJECT_DIR%\protobuf\src
set PROTOC=%PROJECT_DIR%\vcpkg_installed\x64-windows\tools\protobuf\protoc.exe
set GRPC_PLUGIN=%PROJECT_DIR%\vcpkg_installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe


rem 生成 common 模块：：包含共享数据类型，枚举的定义
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\common.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\common.proto

rem 生成 battle 模块：战斗服务器
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_battle.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_battle.proto

rem 生成 chat 模块：聊天服务器
rem %PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_chat.proto
rem %PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_chat.proto

rem 生成 central 模块：中心服务器
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_central.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_central.proto

rem 生成 db 模块：数据库服务器
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_db.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_db.proto

rem 生成 file 模块：文件服务器
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_file.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_file.proto

rem 生成 gateway 模块：网关服务器
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_gateway.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_gateway.proto

rem 生成 logic 模块：逻辑服务器
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_logic.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_logic.proto

rem 生成 login 模块：登录服务器
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_login.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_login.proto

rem 生成 matching 模块：匹配服务器
rem %PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_matching.proto
rem %PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_matching.proto

rem 生成 store 模块：商城服务器
rem %PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_store.proto
rem %PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_store.proto


echo 脚本执行完毕
pause
