#include "central_server.h"

CentralServerImpl::CentralServerImpl()
{
    try {
        std::cerr << "�������ӵ� Redis ������..." << std::endl;
        redis_client.get_client()->connect("127.0.0.1", 6379);
        std::cerr << "�ɹ����ӵ� Redis ������" << std::endl;
    }
    catch (const cpp_redis::redis_error& e) {
        std::cerr << "Redis ���Ӵ���: " << e.what() << std::endl;
        std::abort();
    }
    catch (const std::exception& e) {
        std::cerr << "�����쳣: " << e.what() << std::endl;
        std::abort();
    }
    catch (...) {
        std::cerr << "δ֪�쳣" << std::endl;
        std::abort();
    }
}


CentralServerImpl::~CentralServerImpl()
{
	redis_client.get_client()->disconnect(); // �Ͽ� redis����
}

// ������ע��
grpc::Status CentralServerImpl::register_server(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response) 
{
    std::string server_name = request->server_name();
    std::string address = request->address();
    int port = request->port();

    try {
        redis_client.get_client()->hset("servers", server_name, address + ":" + std::to_string(port));
        redis_client.get_client()->sync_commit();
    }
    catch (const cpp_redis::redis_error& e) {
        std::cerr << "Redis hset error: " << e.what() << std::endl;
        response->set_success(false);
        response->set_message("������ע��ʧ��");
        return grpc::Status::OK;
    }

    response->set_success(true);
    response->set_message("������ע��ɹ�");
    return grpc::Status::OK;
}

// ����Ŀ���������Ϣ
grpc::Status CentralServerImpl::get_server_info(grpc::ServerContext* context, const myproject::ServerInfoRequest* request, myproject::ServerInfoResponse* response) 
{
    std::string server_name = request->server_name();

    // �� Redis ��ȡ��������Ϣ
    auto reply = redis_client.get_client()->hget("servers", server_name);
    redis_client.get_client()->sync_commit();
    auto future = reply.get();

    if (future.is_null()) {
        response->set_success(false);
        response->set_message("������δע��");
    }
    else {
        std::string server_info = future.as_string();
        size_t pos = server_info.find(':');
        std::string address = server_info.substr(0, pos);
        int port = std::stoi(server_info.substr(pos + 1));

        response->set_success(true);
        response->set_address(address);
        response->set_port(port);
        response->set_message("��������Ϣ��ȡ�ɹ�");
    }
    return grpc::Status::OK;
}
