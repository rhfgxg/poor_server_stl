#include "login_service.h"
#include "login_server.h"

grpc::Status LoginServiceImpl::Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response)
{
    // ��ȡ�û���������
    std::string username = request->username(); // �� request ��������ȡ�û���������
    std::string password = request->password();

	// ����������󣬴����¼����
    LoginService login(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials()));

    std::map<std::string, std::string> query = { {"user_name", username}, {"user_password", password} }; // �����ѯ����
	std::string responses = login.login_("poor_users", "users", query); // ��ѯ�����ݿ�������������ѯ����

    if (responses == "��¼�ɹ�")
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