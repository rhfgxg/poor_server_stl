@echo off
rem �������з�����

rem ���õ���Ŀ¼
set DEBUG_DIR=D:\Project\cpp\APP\poor\poor_server_stl\out\build\x64-Debug

rem ���ݿ������
start "DataServer" cmd /k "%DEBUG_DIR%\src\data\data.exe"
rem ��¼������
start "LoginServer" cmd /k "%DEBUG_DIR%\src\login\login.exe"
rem ���ط�����
start "GatewayServer" cmd /k "%DEBUG_DIR%\src\gateway\gateway.exe"

pause