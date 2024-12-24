@echo off
rem 根据 proto文件，生成对应源码


rem 设置路径常量
set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..\..
set PROTO_DIR=%PROJECT_DIR%\protobuf
set PROTOC=%PROJECT_DIR%\vcpkg_installed\x64-windows\tools\protobuf\protoc.exe
set GRPC_PLUGIN=%PROJECT_DIR%\vcpkg_installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe


rem 生成 central 模块：中心模块
set CENTRAL_PROTO_DIR=%PROTO_DIR%\central
%PROTOC% --proto_path=%CENTRAL_PROTO_DIR% --cpp_out=%CENTRAL_PROTO_DIR% %CENTRAL_PROTO_DIR%\server_central.proto
%PROTOC% --proto_path=%CENTRAL_PROTO_DIR% --grpc_out=%CENTRAL_PROTO_DIR% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %CENTRAL_PROTO_DIR%\server_central.proto

rem 生成 data 模块：数据库模块
set DATABASE_PROTO_DIR=%PROTO_DIR%\data
%PROTOC% --proto_path=%DATABASE_PROTO_DIR% --cpp_out=%DATABASE_PROTO_DIR% %DATABASE_PROTO_DIR%\server_data.proto
%PROTOC% --proto_path=%DATABASE_PROTO_DIR% --grpc_out=%DATABASE_PROTO_DIR% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %DATABASE_PROTO_DIR%\server_data.proto

rem 生成 gateway 模块：网关模块
set GATEWAY_PROTO_DIR=%PROTO_DIR%\gateway
%PROTOC% --proto_path=%GATEWAY_PROTO_DIR% --cpp_out=%GATEWAY_PROTO_DIR% %GATEWAY_PROTO_DIR%\server_gateway.proto
%PROTOC% --proto_path=%GATEWAY_PROTO_DIR% --grpc_out=%GATEWAY_PROTO_DIR% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %GATEWAY_PROTO_DIR%\server_gateway.proto

rem 生成 logic 模块：逻辑服务模块
set LOGIC_PROTO_DIR=%PROTO_DIR%\logic
%PROTOC% --proto_path=%LOGIC_PROTO_DIR% --cpp_out=%LOGIC_PROTO_DIR% %LOGIC_PROTO_DIR%\server_logic.proto
%PROTOC% --proto_path=%LOGIC_PROTO_DIR% --grpc_out=%LOGIC_PROTO_DIR% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %LOGIC_PROTO_DIR%\server_logic.proto

rem 生成 login 模块：登录模块
set LOGIN_PROTO_DIR=%PROTO_DIR%\login
%PROTOC% --proto_path=%LOGIN_PROTO_DIR% --cpp_out=%LOGIN_PROTO_DIR% %LOGIN_PROTO_DIR%\server_login.proto
%PROTOC% --proto_path=%LOGIN_PROTO_DIR% --grpc_out=%LOGIN_PROTO_DIR% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %LOGIN_PROTO_DIR%\server_login.proto


echo 脚本执行完毕
pause
