#include "login_service.h"

grpc::Status LoginServiceImpl::Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response) 
{
    /* 
    * �����б�
    * grpc::ServerContext* context��gRPC ����ṩ�������Ķ��󣬰��� RPC ���õ�Ԫ���ݺ�������Ϣ��
    * const myproject::LoginRequest* request���ͻ��˷��͵ĵ�¼���󣬰����û��������롣
    * myproject::LoginResponse* response������˷��ص���Ӧ��������¼�������Ϣ��
    * ����ֵ��
    * gRPC �������
     */

    // ��ȡ�û���������
    std::string username = request->username(); // �� request ��������ȡ�û���������
    std::string password = request->password();

    // �������ݿ��ѯ��У��
    if (username == "user" && password == "pass") 
    {
		response->set_success(true);    // ������Ӧ���� response �� success �ֶ�Ϊ true
        response->set_message("��¼�ɹ�");
    }
    else 
    {
        response->set_success(false);
        response->set_message("��¼ʧ��");
    }

    // ����gRPC״̬
    return grpc::Status::OK;
}