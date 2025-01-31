@echo off
rem 根据 proto文件，生成对应源码


rem 设置路径常量
set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..\..
set PROTO_DIR=%PROJECT_DIR%\protobuf
set MAKE_OUT=%PROJECT_DIR%\protobuf\src
set PROTOC=%PROJECT_DIR%\vcpkg_installed\x64-windows\tools\protobuf\protoc.exe
set GRPC_PLUGIN=%PROJECT_DIR%\vcpkg_installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe

rem 生成共享模块：common：包含共享数据类型，枚举的定义
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\common.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\common.proto

rem 生成 central 模块：中心模块
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_central.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_central.proto

rem 生成 data 模块：数据库模块
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_data.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_data.proto

rem 生成 file 模块：文件服务器模块
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_file.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_file.proto

rem 生成 gateway 模块：网关模块
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_gateway.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_gateway.proto

rem 生成 logic 模块：逻辑服务模块
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_logic.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_logic.proto

rem 生成 login 模块：登录模块
%PROTOC% --proto_path=%PROTO_DIR% --cpp_out=%MAKE_OUT% %PROTO_DIR%\server_login.proto
%PROTOC% --proto_path=%PROTO_DIR% --grpc_out=%MAKE_OUT% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %PROTO_DIR%\server_login.proto


echo 脚本执行完毕
pause
