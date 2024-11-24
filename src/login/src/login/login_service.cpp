#include "./login_service.h"

// ���캯������ʼ�� stub_
LoginService::LoginService(std::shared_ptr<grpc::Channel> channel) : 
    stub_(myproject::DatabaseService::NewStub(channel))
{

}

// Login �����������¼����
std::string LoginService::login_(const std::string& database, const std::string& table, std::map<std::string, std::string> query)
{
    myproject::ReadRequest request;    // �������
    request.set_database(database); // ���ò�ѯ���ݿ�
	request.set_table(table); // ���ò�ѯ��
	for (auto& it : query)
	{
		(*request.mutable_query())[it.first] = it.second; // ���ò�ѯ����
	}

    myproject::ReadResponse response;  // ���ز���
    grpc::ClientContext context;    // ���� RPC ���õ�Ԫ���ݺ�������Ϣ��

    grpc::Status status = stub_->Read(&context, request, &response); // �����ݿ���������Ͳ�ѯ����

    if (status.ok())
    {
        // �����ѯ���
		std::cout << "��¼�ɹ�" << std::endl;
        return "��¼�ɹ�"; // ������Ӧ��Ϣ
    }
    else
    {
		std::cout << "��¼ʧ��" << std::endl;
        return "��¼ʧ��"; // ���ش�����Ϣ
    }
}
