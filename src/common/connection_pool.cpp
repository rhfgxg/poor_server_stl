#include "connection_pool.h"

ConnectionPool::ConnectionPool(size_t pool_size) :
    pool_size(pool_size)
{

}

ConnectionPool::~ConnectionPool() 
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // ����
    for (auto& pair : pool_map) // �������ӳ�
    {
		while (!pair.second.empty())    // ������ӳز�Ϊ��
        {
			pair.second.pop();  // ��������
        }
    }
}

// �������
void ConnectionPool::add_server(myproject::ServerType server_type, const std::string& server_address, const std::string& server_port)
{
	std::lock_guard<std::mutex> lock(pool_mutex);   // ����
	server_info_map[server_type] = { server_address, server_port }; // �����������Ϣ
	for (size_t i = 0; i < pool_size; ++i)  // �������ӳأ��������
    {
		auto channel = new_connection(server_address, server_port); // ��������
		pool_map[server_type].push(channel);    // �������ӳ�
    }
}

// ɾ��ָ��������������
void ConnectionPool::remove_server(myproject::ServerType server_type, const std::string& server_address, const std::string& server_port)
{
	std::lock_guard<std::mutex> lock(pool_mutex);   // ����
	server_info_map.erase(server_type); // ɾ����������Ϣ
	while (!pool_map[server_type].empty())    // ������ӳز�Ϊ��
	{
		pool_map[server_type].pop();    // ��������
	}
}

// ��ȡ����
std::shared_ptr<grpc::Channel> ConnectionPool::get_connection(myproject::ServerType server_type)
{
	std::lock_guard<std::mutex> lock(pool_mutex);   // ����
	if (pool_map[server_type].empty())  // ������ӳ�Ϊ��
    {
		auto& server_info = server_info_map[server_type];   // ��ȡ��������Ϣ
		return new_connection(server_info.first, server_info.second);   // ����������
    }
	auto connection = pool_map[server_type].front();    // ��ȡ����
	pool_map[server_type].pop();    // ��������
    return connection;
}

// �ͷ�����
void ConnectionPool::release_connection(myproject::ServerType server_type, std::shared_ptr<grpc::Channel> connection)
{
	std::lock_guard<std::mutex> lock(pool_mutex);   // ����
	pool_map[server_type].push(connection);  // �������ӳ�
}

// �����ķ�������ȡ��������
std::shared_ptr<grpc::Channel> ConnectionPool::new_connection(const std::string& server_address, const std::string& server_port)
{
    return grpc::CreateChannel(server_address + ":" + server_port, grpc::InsecureChannelCredentials());
}

// �������ӳ��е�����
void ConnectionPool::update_connections(myproject::ServerType server_type, const std::string& server_address, const std::string& server_port)
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // ����

    // ���·�������Ϣ
    server_info_map[server_type] = { server_address, server_port };

    // ��յ�ǰ���ӳ�
    while (!pool_map[server_type].empty()) {
        pool_map[server_type].pop();
    }

    // ��������ӵ����ӳ�
    for (size_t i = 0; i < pool_size; ++i) {
        auto channel = new_connection(server_address, server_port);
        pool_map[server_type].push(channel);
    }
}

// ��ȡ�������ӳص�״̬
std::unordered_map<myproject::ServerType, std::vector<std::pair<std::string, std::string>>> ConnectionPool::get_all_connections()
{
	std::lock_guard<std::mutex> lock(pool_mutex);   // ����
	
	// ������������
	std::unordered_map<myproject::ServerType, std::vector<std::pair<std::string, std::string>>> all_connections;  
	
	for (const auto& pair : server_info_map)   // ������������Ϣ
	{
		all_connections[pair.first].push_back(pair.second);    // ����������Ϣ ��������
	}

    return all_connections;
}
