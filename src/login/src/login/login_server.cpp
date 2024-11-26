#include "login_service.h"
#include "login_server.h"

// ��¼����ӿ�
grpc::Status LoginServerImpl::Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response)
{
    // ��ȡ�û���������
    std::string username = request->username(); // �� request ��������ȡ�û���������
    std::string password = request->password();

	// ����������󣬴����¼����
    LoginService login_service(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials()));

    std::map<std::string, std::string> query = { {"user_name", username}, {"user_password", password} }; // �����ѯ����
	std::string responses = login_service.login_("poor_users", "users", query); // ��ѯ�����ݿ�������������ѯ����

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

// ע�����ӿ�
grpc::Status LoginServerImpl::Register(grpc::ServerContext* context, const myproject::RegisterRequest* request, myproject::RegisterResponse* response)
{
    // ��ȡ�û���������
    std::string username = request->username(); // �� request ��������ȡ�û���������
    std::string password = request->password();
	std::string email = request->email();

    // ����������󣬴���ע������
    RegisterService register_service(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials()));

    std::map<std::string, std::string> data = { {"user_name", username}, {"user_password", password} }; // ������������
    std::string responses = register_service.register_("poor_users", "users", data); // ��ѯ�����ݿ�������������������

    if (responses == "���ӳɹ�")
    {
        response->set_success(true);    // ������Ӧ���� response �� success �ֶ�Ϊ true
        response->set_message("ע��ɹ�");
    }
    else
    {
        response->set_success(false);
        response->set_message("ע��ʧ��");
    }

    // ����gRPC״̬
    return grpc::Status::OK;
}

// ������֤����ӿ�
grpc::Status LoginServerImpl::Authenticate(grpc::ServerContext* context, const myproject::AuthenticateRequest* request, myproject::AuthenticateResponse* response)
{
    // ��ȡ�û���������
    std::string token = request->token(); // �� request ��������ȡ�û���������

	// ����������󣬴���������֤����
    AuthenticateService authenticate_server;
    std::string responses = authenticate_server.authenticate_(token);

    if (responses == "��֤��ȷ")
    {
        response->set_success(true);    // ������Ӧ���� response �� success �ֶ�Ϊ true
        response->set_message("��֤��ȷ");
    }
    else
    {
        response->set_success(false);
        response->set_message("��֤����");
    }

    // ����gRPC״̬
    return grpc::Status::OK;
}