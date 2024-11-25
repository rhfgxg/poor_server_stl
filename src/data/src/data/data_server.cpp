#include "data_server.h"

DatabaseServiceImpl::DatabaseServiceImpl(mysqlx::Session& DBlink_) :
	DBlink(DBlink_)
{

}

DatabaseServiceImpl::~DatabaseServiceImpl()
{

}

grpc::Status DatabaseServiceImpl::Create(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response)
{
	// ��ȡ�������
	std::string db_name = request->database();
	std::string tab_name = request->table();
	// �������ݿ����
	// ...
	// ������Ӧ����
	response->set_success(true);
	response->set_message("�����ɹ�");
	std::cout << "���ݿⴴ���ɹ�" << std::endl;

	return grpc::Status::OK;
}

grpc::Status DatabaseServiceImpl::Read(grpc::ServerContext* context, const myproject::ReadRequest* request, myproject::ReadResponse* response)
{
    try 
    {
        // ��ȡ����� query ����
        const std::string& database = request->database();
        const std::string& table = request->table();
        const auto& query = request->query();

        // ѡ�����ݿ�
        mysqlx::Table tbl = this->DBlink.getSchema(database).getTable(table);

        // ������ѯ����
        std::string condition;
        for (const auto& q : query) 
        {
            if (!condition.empty()) 
            {
                condition += " AND ";
            }
            condition += q.first + " = '" + q.second + "'";

            std::cout << condition;
        }


        response->set_success(true);
        response->set_message("��ѯ�ɹ�");
        return grpc::Status::OK;
    }
    catch (const mysqlx::Error& err) 
    {
        std::cerr << "���ݿ��ѯ����: " << err.what() << std::endl;
        response->set_success(false);
        response->set_message("���ݿ��ѯ����");
        return grpc::Status::CANCELLED;
    }
    catch (std::exception& ex) 
    {
        std::cerr << "�쳣: " << ex.what() << std::endl;
        response->set_success(false);
        response->set_message("�쳣");
        return grpc::Status::CANCELLED;
    }
    catch (...) 
    {
        std::cerr << "δ֪����!" << std::endl;
        response->set_success(false);
        response->set_message("δ֪����");
        return grpc::Status::CANCELLED;
    }
}

grpc::Status DatabaseServiceImpl::Update(grpc::ServerContext* context, const myproject::UpdateRequest* request, myproject::UpdateResponse* response)
{
	// ��ȡ�������
	std::string db_name = request->database();
	std::string tab_name = request->table();
	// �������ݿ����
	// ...
	// ������Ӧ����
	response->set_success(true);
	response->set_message("���³ɹ�");
	std::cout << "���ݿ���³ɹ�" << std::endl;

	return grpc::Status::OK;
}

grpc::Status DatabaseServiceImpl::Delete(grpc::ServerContext* context, const myproject::DeleteRequest* request, myproject::DeleteResponse* response)
{
	// ��ȡ�������
	std::string db_name = request->database();
	std::string tab_name = request->table();
	// �������ݿ����
	// ...
	// ������Ӧ����
	response->set_success(true);
	response->set_message("ɾ���ɹ�");
	std::cout << "���ݿ�ɾ���ɹ�" << std::endl;

	return grpc::Status::OK;
}