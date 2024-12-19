#ifndef DATA_SERVICE_H
#define DATA_SERVICE_H

#include "server_data.grpc.pb.h"
#include "server_central.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <mysqlx/xdevapi.h> // mysql

// 数据库服务实现类
class DatabaseServerImpl final : public myproject::DatabaseServer::Service {
public:
	explicit DatabaseServerImpl(mysqlx::Session& DBlink_); // 构造函数：传入数据库链接
    ~DatabaseServerImpl() override; // 添加析构函数声明

	void register_server(); // 注册服务器
	void unregister_server(); // 注销服务器

	// 服务实现
	// 添加数据库记录
    grpc::Status Create(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response) override;
	// 读取数据库记录
    grpc::Status Read(grpc::ServerContext* context, const myproject::ReadRequest* request, myproject::ReadResponse* response) override;
	// 更新数据库记录
	grpc::Status Update(grpc::ServerContext* context, const myproject::UpdateRequest* request, myproject::UpdateResponse* response) override;
	// 删除数据库记录
	grpc::Status Delete(grpc::ServerContext* context, const myproject::DeleteRequest* request, myproject::DeleteResponse* response) override;

private:
	mysqlx::Session& DBlink; // 数据库链接

	std::unique_ptr<myproject::CentralServer::Stub> central_stub;	// 中心服务存根
};

#endif // DATA_SERVICE_H