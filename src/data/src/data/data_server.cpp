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
	// 获取请求参数
	std::string db_name = request->database();
	std::string tab_name = request->table();
	// 假设数据库操作
	// ...
	// 设置响应参数
	response->set_success(true);
	response->set_message("创建成功");
	std::cout << "数据库创建成功" << std::endl;

	return grpc::Status::OK;
}

grpc::Status DatabaseServiceImpl::Read(grpc::ServerContext* context, const myproject::ReadRequest* request, myproject::ReadResponse* response)
{
    try 
    {
        // 获取请求的 query 参数
        const std::string& database = request->database();
        const std::string& table = request->table();
        const auto& query = request->query();

        // 选择数据库
        mysqlx::Table tbl = this->DBlink.getSchema(database).getTable(table);

        // 构建查询条件
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
        response->set_message("查询成功");
        return grpc::Status::OK;
    }
    catch (const mysqlx::Error& err) 
    {
        std::cerr << "数据库查询错误: " << err.what() << std::endl;
        response->set_success(false);
        response->set_message("数据库查询错误");
        return grpc::Status::CANCELLED;
    }
    catch (std::exception& ex) 
    {
        std::cerr << "异常: " << ex.what() << std::endl;
        response->set_success(false);
        response->set_message("异常");
        return grpc::Status::CANCELLED;
    }
    catch (...) 
    {
        std::cerr << "未知错误!" << std::endl;
        response->set_success(false);
        response->set_message("未知错误");
        return grpc::Status::CANCELLED;
    }
}

grpc::Status DatabaseServiceImpl::Update(grpc::ServerContext* context, const myproject::UpdateRequest* request, myproject::UpdateResponse* response)
{
	// 获取请求参数
	std::string db_name = request->database();
	std::string tab_name = request->table();
	// 假设数据库操作
	// ...
	// 设置响应参数
	response->set_success(true);
	response->set_message("更新成功");
	std::cout << "数据库更新成功" << std::endl;

	return grpc::Status::OK;
}

grpc::Status DatabaseServiceImpl::Delete(grpc::ServerContext* context, const myproject::DeleteRequest* request, myproject::DeleteResponse* response)
{
	// 获取请求参数
	std::string db_name = request->database();
	std::string tab_name = request->table();
	// 假设数据库操作
	// ...
	// 设置响应参数
	response->set_success(true);
	response->set_message("删除成功");
	std::cout << "数据库删除成功" << std::endl;

	return grpc::Status::OK;
}