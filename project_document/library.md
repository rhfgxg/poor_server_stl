# ��Ŀ��ʹ�õĵ�������
���е��������ļ������� vcpkg_installed/x64-windwos �ļ����£���Ҫʹ��vcpkg��װ���Զ����ɣ�
��������� debugģʽ���ļ������� vcpkg_installed/x64-windwos/debug �ļ�����

## ���������б�
������ֱ��ʹ�õĿ��б�

### �����
boost��

### grpc���ͻ������������������֮���ͨ��Э��
grpc��
ʵ�ַ�����֮���Զ�̵���

### mysql��
mysql-connector-cpp��
ʹ��mysqlxЭ�飬�����������ݿ�Ķ˿���33060

### redis�������
cpp-redis��
tacopie��
redis��windows�¹���ʱ���ᵼ��һЩ�쳣��������Ҫʹ������а�װ��Ȼ��ʹ����ķ���������redis����� central_server.h��

### json����
nlohmann-json��

### protobuf���ͻ������������������֮����������л�
protobuf�⣺������ grpc����

�����ļ��õ� protoc������������ļ��� /vcpkg_installed/x64-windows/tools/protobuf/protoc.exe
���� rpc�����ļ�ʱ����Ҫ�õ� grpc_cpp_plugin.exe������ļ���Ҫ�Ȱ�װ grpc�⣬��װ�󱣴��� /vcpkg_installed/x64-windows/tools/grpc/grpc_cpp_plugin.exe

������ʹ�� whichָ��Ѱ�� grpc_cpp_plugin����ʱ��һֱ�Ҳ���������ʹ�õ������·��
proto �У�stringĬ���� utf-8��ʽ����c++��ͻ�������ڱ�д proto�ļ�ʱ����Ҫʹ�� bytes����string
