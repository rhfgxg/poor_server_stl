@echo off
rem �������з�����

rem ���õ���Ŀ¼
set DEBUG_DIR=D:\Project\cpp\APP\poor\poor_server_stl\out\install\x64-Debug

rem ���ķ�����
start "CentralServer" cmd /k "%DEBUG_DIR%\bin\central.exe"
rem ���ݿ������
start "DataServer" cmd /k "%DEBUG_DIR%\bin\data.exe"
rem ��¼������
start "LoginServer" cmd /k "%DEBUG_DIR%\bin\login.exe"
rem �߼�������
rem start "LogicServer" cmd /k "%DEBUG_DIR%\bin\logic.exe"
rem ���ط�����
start "GatewayServer" cmd /k "%DEBUG_DIR%\bin\gateway.exe"

pause