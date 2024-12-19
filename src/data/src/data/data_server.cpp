#include "data_server.h"
#include <codecvt>
#include <locale>

DatabaseServerImpl::DatabaseServerImpl(mysqlx::Session& DBlink_) :
	DBlink(DBlink_),    // 数据库链接
    central_stub(myproject::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))) // 中心服务器存根
{

}

DatabaseServerImpl::~DatabaseServerImpl()
{

}

// 注册服务器
void DatabaseServerImpl::register_server()
{
    // 请求
    myproject::RegisterServerRequest request;
    request.set_server_type(myproject::ServerType::DATA);
    request.set_address("127.0.0.1");
    request.set_port("50052");
    // 响应
    myproject::RegisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->RegisterServer(&context, request, &response);

    if (status.ok() && response.success())
    {
        std::cout << "服务器注册成功: " << response.message() << std::endl;
    }
    else
    {
        std::cerr << "服务器注册失败: " << response.message() << std::endl;
    }
}

// 注销服务器
void DatabaseServerImpl::unregister_server()
{
    // 请求
    myproject::UnregisterServerRequest request;
    request.set_server_type(myproject::ServerType::DATA);
    request.set_address("localhost");
    request.set_port("50052");

    // 响应
    myproject::UnregisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->UnregisterServer(&context, request, &response);

    if (status.ok() && response.success()) 
    {
        std::cout << "服务器注销成功: " << response.message() << std::endl;
    }
    else
    {
        std::cerr << "服务器注销失败: " << response.message() << std::endl;
    }
}

// 添加
grpc::Status DatabaseServerImpl::Create(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response)
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

// 查询
grpc::Status DatabaseServerImpl::Read(grpc::ServerContext* context, const myproject::ReadRequest* request, myproject::ReadResponse* response)
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
        }

        // 执行查询
        mysqlx::RowResult result = tbl.select("*").where(condition).execute();;

        // 设置响应：查询结果
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
                    column_value = "Raw Data"; // 需要进一步处理原始数据
                }
                else {
                    column_value = "Unsupported Type";
                }
                response_result->mutable_fields()->insert({ column_name, column_value });
            }
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

// 更新
grpc::Status DatabaseServerImpl::Update(grpc::ServerContext* context, const myproject::UpdateRequest* request, myproject::UpdateResponse* response)
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

// 删除
grpc::Status DatabaseServerImpl::Delete(grpc::ServerContext* context, const myproject::DeleteRequest* request, myproject::DeleteResponse* response)
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