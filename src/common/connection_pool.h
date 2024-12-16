#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include "server_central.grpc.pb.h" // ʹ�÷���������ö��

#include <grpcpp/grpcpp.h>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <unordered_map>

class ConnectionPool {
public:
    ConnectionPool(size_t pool_size);
    ~ConnectionPool();

    // �����ӳ����������
    void add_server(myproject::ServerType server_type, const std::string& server_address, const std::string& server_port);
	// ɾ��ָ��������������
    void remove_server(myproject::ServerType server_type, const std::string& server_address, const std::string& server_port);

    // ��ȡ����
    std::shared_ptr<grpc::Channel> get_connection(myproject::ServerType server_type);
    // �ͷ�����
    void release_connection(myproject::ServerType server_type, std::shared_ptr<grpc::Channel> connection);

    // �������ӳ��е�����
    void update_connections(myproject::ServerType server_type, const std::string& server_address, const std::string& server_port);

    // ��ȡ�������ӳص�״̬
    std::unordered_map<myproject::ServerType, std::vector<std::pair<std::string, std::string>>> get_all_connections();

private:
    // �����ķ�������ȡ��������
    std::shared_ptr<grpc::Channel> new_connection(const std::string& server_address, const std::string& server_port);

    size_t pool_size;  // ���ӳش�С
    std::unordered_map<myproject::ServerType, std::queue<std::shared_ptr<grpc::Channel>>> pool_map; // ���ӳ�ӳ��
    std::unordered_map<myproject::ServerType, std::pair<std::string, std::string>> server_info_map; // ��������Ϣӳ��
    std::mutex pool_mutex; // ���ӳػ�����
};

#endif // CONNECTION_POOL_H
