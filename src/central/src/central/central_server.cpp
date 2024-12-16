#include "central_server.h"

CentralServerImpl::CentralServerImpl() :
    central_connection_pool(10),
	data_connection_pool(10),   // ���ݿ����ӳش�СΪ 10
	gateway_connection_pool(10),    // �������ӳش�СΪ 10
	login_connection_pool(10)   // ��¼���ӳش�СΪ 10
{
    // �����ķ��������ӳ��м��뱾������
    central_connection_pool.add_server(myproject::ServerType::CENTRAL, "localhost", "50050");
    std::cout << "���ķ�����ע��ɹ�: " << " " << "localhost" << " " << "50050" << std::endl;

    //// ������ʱ���񣬶�ʱ������������״̬
    //std::thread([this]() {
    //    while (true) {
    //        // ������������״̬
    //        // �������ӳ�
    //        std::this_thread::sleep_for(std::chrono::seconds(60));
    //    }
    //    }).detach();
}

CentralServerImpl::~CentralServerImpl()
{
}

// ������ע��
grpc::Status CentralServerImpl::RegisterServer(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response)
{
	myproject::ServerType server_type = request->server_type(); // ����������
	std::string address = request->address();   // ��������ַ
	std::string port = request->port(); // �������˿�

    // ���ݷ��������ͣ����������������ӳ�
    switch (server_type)
    {
    case myproject::ServerType::CENTRAL:
    {
        // todo�����ķ�����
        std::cout << "��δʵ��" << std::endl;
        break;
    }
	case myproject::ServerType::DATA:   // ���ݿ������
    {
		data_connection_pool.add_server(myproject::ServerType::DATA, address, port);    // �������ӳ�
        std::cout << "���ݿ������ע��ɹ�: " << " " << address << " " << port << std::endl;
        break;
    }
	case myproject::ServerType::GATEWAY:    // ���ط�����
    {
        gateway_connection_pool.add_server(myproject::ServerType::GATEWAY, address, port);
        std::cout << "���ط�����ע��ɹ�: " << " " << address << " " << port << std::endl;
        break;
    }
	case myproject::ServerType::LOGIN:      // ��¼������
    {
        login_connection_pool.add_server(myproject::ServerType::LOGIN, address, port);
        std::cout << "��¼������ע��ɹ�: " << " " << address << " " << port << std::endl;
        break;
    }
    }

    response->set_success(true);
    response->set_message("������ע��ɹ�");
    return grpc::Status::OK;
}

// �Ͽ�����������
grpc::Status CentralServerImpl::UnregisterServer(grpc::ServerContext* context, const myproject::UnregisterServerRequest* request, myproject::UnregisterServerResponse* response)
{
    myproject::ServerType server_type = request->server_type(); // ����������  
    std::string address = request->address();   // ��������ַ
    std::string port = request->port(); // �������˿�

    // ���ݷ��������ͣ������ӳ���ɾ��������
    switch (server_type)
    {
    case myproject::ServerType::CENTRAL:
    {
        // todo�����ķ�����
        std::cout << "��δʵ��" << std::endl;
        break;
    }
    case myproject::ServerType::DATA:   // ���ݿ������
    {
        data_connection_pool.remove_server(myproject::ServerType::DATA, address, port);    // �����ӳ���ɾ��
        std::cout << "���ݿ�������Ͽ����ӳɹ�: " << " " << address << " " << port << std::endl;
        break;
    }
    case myproject::ServerType::GATEWAY:    // ���ط�����
    {
        gateway_connection_pool.remove_server(myproject::ServerType::GATEWAY, address, port);
        std::cout << "���ط������Ͽ����ӳɹ�: " << " " << address << " " << port << std::endl;
        break;
    }
    case myproject::ServerType::LOGIN:      // ��¼������
    {
        login_connection_pool.remove_server(myproject::ServerType::LOGIN, address, port);
        std::cout << "��¼�������Ͽ����ӳɹ�: " << " " << address << " " << port << std::endl;
        break;
    }
    default:
        response->set_success(false);
        response->set_message("��Ч�ķ���������");
        return grpc::Status::OK;
    }

    std::cout << "�������Ͽ����ӳɹ�: " << std::endl;
    response->set_success(true);
    response->set_message("�������Ͽ����ӳɹ�");

    return grpc::Status::OK;
}

// ��ȡ���ӳ�����������
grpc::Status CentralServerImpl::GetConnectPoor(grpc::ServerContext* context, const myproject::ConnectPoorRequest* request, myproject::ConnectPoorResponse* response)
{
	std::cout << "��ȡ���ӳ���Ϣ" << std::endl;
    myproject::ServerType server_type = request->server_type(); // ����������  

    std::unordered_map<myproject::ServerType, std::vector<std::pair<std::string, std::string>>> connections;

    switch (server_type)
    {
    case myproject::CENTRAL:
    {
        // ��ȡ������������ӳ���Ϣ
        connections = central_connection_pool.get_all_connections();
        break;
    }
    case myproject::DATA:
    {
        // ��ȡ���ݿ���������ӳ���Ϣ
        connections = data_connection_pool.get_all_connections();
        break;
    }
    case myproject::GATEWAY:
    {
        // ��ȡ���ط��������ӳ���Ϣ
        connections = gateway_connection_pool.get_all_connections();
        break;
    }
    //case myproject::LOGIC: 
    //{
    //    // ��ȡ�߼����������ӳ���Ϣ
    //    connections = logic_connection_pool.get_all_connections();
    //    break;
    //}
    case myproject::LOGIN:
    {
        // ��ȡ��¼���������ӳ���Ϣ
        connections = login_connection_pool.get_all_connections();
        break;
    }
    default:
        response->set_success(false);
        response->set_message("��Ч�ķ���������");
        return grpc::Status::OK;
    }

    for (const auto& connection : connections[server_type])
    {
        auto* conn_info = response->add_connect_info();
        conn_info->set_address(connection.first);
        conn_info->set_port(std::stoi(connection.second)); // ���˿ڴ��ַ���ת��Ϊ����
    }

    response->set_success(true);
    response->set_message("��ȡ���ӳ���Ϣ�ɹ�");
    return grpc::Status::OK;
}

