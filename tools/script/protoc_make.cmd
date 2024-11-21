@echo off

rem ����·������
set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..\..
set PROTOC=%PROJECT_DIR%\vcpkg_installed\x64-windows\tools\protobuf\protoc.exe
set GRPC_PLUGIN=%PROJECT_DIR%\vcpkg_installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe

rem ���� central ģ��� protobuf�� grpc ����
set CENTRAL_PROTO_DIR=%PROJECT_DIR%\protobuf\central
%PROTOC% --proto_path=%CENTRAL_PROTO_DIR% --cpp_out=%CENTRAL_PROTO_DIR% %CENTRAL_PROTO_DIR%\server_central.proto
%PROTOC% --proto_path=%CENTRAL_PROTO_DIR% --grpc_out=%CENTRAL_PROTO_DIR% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %CENTRAL_PROTO_DIR%\server_central.proto

rem ���� data ģ��� protobuf �� gRPC ����
set DATABASE_PROTO_DIR=%PROJECT_DIR%\protobuf\data
%PROTOC% --proto_path=%DATABASE_PROTO_DIR% --cpp_out=%DATABASE_PROTO_DIR% %DATABASE_PROTO_DIR%\server_data.proto
%PROTOC% --proto_path=%DATABASE_PROTO_DIR% --grpc_out=%DATABASE_PROTO_DIR% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %DATABASE_PROTO_DIR%\server_data.proto

rem ���� gateway ģ��� protobuf �� gRPC ����
set GATEWAY_PROTO_DIR=%PROJECT_DIR%\protobuf\gateway
%PROTOC% --proto_path=%GATEWAY_PROTO_DIR% --cpp_out=%GATEWAY_PROTO_DIR% %GATEWAY_PROTO_DIR%\server_gateway.proto
%PROTOC% --proto_path=%GATEWAY_PROTO_DIR% --grpc_out=%GATEWAY_PROTO_DIR% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %GATEWAY_PROTO_DIR%\server_gateway.proto

rem ���� logic ģ��� protobuf �� gRPC ����
set LOGIC_PROTO_DIR=%PROJECT_DIR%\protobuf\logic
%PROTOC% --proto_path=%LOGIC_PROTO_DIR% --cpp_out=%LOGIC_PROTO_DIR% %LOGIC_PROTO_DIR%\server_logic.proto
%PROTOC% --proto_path=%LOGIC_PROTO_DIR% --grpc_out=%LOGIC_PROTO_DIR% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %LOGIC_PROTO_DIR%\server_logic.proto

rem ���� login ģ��� protobuf �� gRPC ����
set LOGIN_PROTO_DIR=%PROJECT_DIR%\protobuf\login
%PROTOC% --proto_path=%LOGIN_PROTO_DIR% --cpp_out=%LOGIN_PROTO_DIR% %LOGIN_PROTO_DIR%\server_login.proto
%PROTOC% --proto_path=%LOGIN_PROTO_DIR% --grpc_out=%LOGIN_PROTO_DIR% --plugin=protoc-gen-grpc=%GRPC_PLUGIN% %LOGIN_PROTO_DIR%\server_login.proto

rem �ű�ִ�����
echo �ű�ִ�����
pause
