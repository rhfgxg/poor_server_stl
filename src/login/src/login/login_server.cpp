#include "login_server.h"

LoginServerImpl::LoginServerImpl() :
    central_stub(myproject::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))), // �������ķ�����
	db_connection_pool(10) // ��ʼ�����ݿ���������ӳأ��������ӳش�СΪ10
{

}

// ע�������
void LoginServerImpl::register_server() 
{
    // ����
    myproject::RegisterServerRequest request;
    request.set_server_type(myproject::ServerType::LOGIN);
    request.set_address("127.0.0.1");
    request.set_port("50053");

    // ��Ӧ
    myproject::RegisterServerResponse response;

    // �ͻ���
    grpc::ClientContext context;

    grpc::Status status = central_stub->RegisterServer(&context, request, &response);

    if (status.ok() && response.success())
    {
        std::cout << "������ע��ɹ�: " << response.message() << std::endl;

		init_connection_pool(); // ��ʼ�����ӳ�
    }
    else 
    {
        std::cerr << "������ע��ʧ��: " << response.message() << std::endl;
    }
}

// ע��������
void LoginServerImpl::unregister_server() 
{
    // ����
    myproject::UnregisterServerRequest request;
    request.set_server_type(myproject::ServerType::LOGIN);
    request.set_address("localhost");
    request.set_port("50053");

    // ��Ӧ
    myproject::UnregisterServerResponse response;

    // �ͻ���
    grpc::ClientContext context;

    grpc::Status status = central_stub->UnregisterServer(&context, request, &response);

    if (status.ok() && response.success()) {
        std::cout << "������ע���ɹ�: " << response.message() << std::endl;
    }
    else {
        std::cerr << "������ע��ʧ��: " << response.message() << std::endl;
    }
}

// ��ʼ�����ӳ�
void LoginServerImpl::init_connection_pool()
{
    // �ͻ���
    grpc::ClientContext context;
    // ����
    myproject::ConnectPoorRequest request;
    request.set_server_type(myproject::ServerType::DATA);
    // ��Ӧ
    myproject::ConnectPoorResponse response;

    grpc::Status status = central_stub->GetConnectPoor(&context, request, &response);

    if (status.ok())
    {
        // ���µ�¼���������ӳ�
        for (const auto& server_info : response.connect_info())
        {
            db_connection_pool.add_server(myproject::ServerType::DATA, server_info.address(), std::to_string(server_info.port()));
        }
    }
    else
    {
        std::cerr << "�޷���ȡ��¼���������ӳ���Ϣ: " << status.error_message() << std::endl;
    }
}


// ��¼����ӿ�
grpc::Status LoginServerImpl::Login(grpc::ServerContext* context, const myproject::LoginRequest* request, myproject::LoginResponse* response)
{
    // ��ȡ�û���������
    std::string username = request->username(); // �� request ��������ȡ�û���������
    std::string password = request->password();
	// �����ѯ����
    std::map<std::string, std::string> query = { {"user_name", username}, {"user_password", password} }; 

	std::string responses = login_("poor_users", "users", query); // ��ѯ�����ݿ�������������ѯ����

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

    // ����gRPC״̬
    return grpc::Status::OK;
}

// ������֤����ӿ�
grpc::Status LoginServerImpl::Authenticate(grpc::ServerContext* context, const myproject::AuthenticateRequest* request, myproject::AuthenticateResponse* response)
{

    return grpc::Status::OK;
}


// ��¼����
std::string LoginServerImpl::login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query)
{
    // ��������
    myproject::ReadRequest read_request;
    read_request.set_database(database); // ���ò�ѯ���ݿ�
    read_request.set_table(table); // ���ò�ѯ��
    for (auto& it : query)
    {
        (*read_request.mutable_query())[it.first] = it.second; // ���ò�ѯ����
    }

    // ������Ӧ
    myproject::ReadResponse read_response;
    grpc::ClientContext client_context; // ���� RPC ���õ�Ԫ���ݺ�������Ϣ

    // ��ȡ���ӳ��е�����
    auto channel = db_connection_pool.get_connection(myproject::ServerType::DATA);
    auto db_stub = myproject::DatabaseServer::NewStub(channel);

    // �����ݿ���������Ͳ�ѯ����
    grpc::Status status = db_stub->Read(&client_context, read_request, &read_response);

    if (status.ok())
    {
        // ��ӡ��ѯ���
        for (const auto& result : read_response.results())
        {
            std::cout << "��ѯ���: ";
            for (const auto& field : result.fields())
            {
                std::cout << field.first << ": " << field.second << ", ";
            }
            std::cout << std::endl;
        }

        std::cout << "��¼�ɹ�" << std::endl;
        return "��¼�ɹ�";
    }
    else
    {
        std::cout << "��¼ʧ��" << std::endl;
        return "��¼ʧ��";
    }
}

// ע�����
std::string LoginServerImpl::register_(const std::string& database, const std::string& table, std::map<std::string, std::string> data)
{
	return "ע��ɹ�";
}

// ������֤����
std::string LoginServerImpl::authenticate_(const std::string& token)
{
	return "��֤�ɹ�";
}