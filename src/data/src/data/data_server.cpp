#include "data_server.h"
#include <codecvt>
#include <locale>

DatabaseServerImpl::DatabaseServerImpl(mysqlx::Session& DBlink_) :
	DBlink(DBlink_)
{

}

DatabaseServerImpl::~DatabaseServerImpl()
{

}

// ���
grpc::Status DatabaseServerImpl::Create(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response)
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

// ��ѯ
grpc::Status DatabaseServerImpl::Read(grpc::ServerContext* context, const myproject::ReadRequest* request, myproject::ReadResponse* response)
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
        }

        // ִ�в�ѯ
        mysqlx::RowResult result = tbl.select("*").where(condition).execute();;

        // ������Ӧ����ѯ���
        for (mysqlx::Row row : result)
        {
            myproject::ReadResponse::Result* response_result = response->add_results();
            for (size_t i = 0; i < row.colCount(); ++i)
            {
                std::string column_name = result.getColumn(i).getColumnName();
                std::string column_value;
                if (row[i].isNull())
                {
                    column_value = "NULL";
                }
                else if (row[i].getType() == mysqlx::Value::Type::STRING) {
                    column_value = row[i].get<std::string>();
                }
                else if (row[i].getType() == mysqlx::Value::Type::VNULL) {
                    column_value = "VNULL";
                }
                else if (row[i].getType() == mysqlx::Value::Type::INT64) {
                    column_value = std::to_string(row[i].get<int64_t>());
                }
                else if (row[i].getType() == mysqlx::Value::Type::UINT64) {
                    column_value = std::to_string(row[i].get<uint64_t>());
                }
                else if (row[i].getType() == mysqlx::Value::Type::FLOAT) {
                    column_value = std::to_string(row[i].get<float>());
                }
                else if (row[i].getType() == mysqlx::Value::Type::DOUBLE) {
                    column_value = std::to_string(row[i].get<double>());
                }
                else if (row[i].getType() == mysqlx::Value::Type::BOOL) {
                    column_value = row[i].get<bool>() ? "true" : "false";
                }
                else if (row[i].getType() == mysqlx::Value::Type::RAW) {
                    column_value = "Raw Data"; // ��Ҫ��һ������ԭʼ����
                }
                else {
                    column_value = "Unsupported Type";
                }
                response_result->mutable_fields()->insert({ column_name, column_value });
            }
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

// ����
grpc::Status DatabaseServerImpl::Update(grpc::ServerContext* context, const myproject::UpdateRequest* request, myproject::UpdateResponse* response)
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

// ɾ��
grpc::Status DatabaseServerImpl::Delete(grpc::ServerContext* context, const myproject::DeleteRequest* request, myproject::DeleteResponse* response)
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