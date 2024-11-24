#ifndef DATA_SERVICE_H
#define DATA_SERVICE_H

#include "server_data.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <mysqlx/xdevapi.h> // mysql

// 数据库服务实现类
class DatabaseServiceImpl final : public myproject::DatabaseService::Service {
public:
	explicit DatabaseServiceImpl(mysqlx::Session& DBlink_); // 构造函数：传入数据库链接
    ~DatabaseServiceImpl() override; // 添加析构函数声明

	// 服务实现
    grpc::Status Create(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response) override;
    grpc::Status Read(grpc::ServerContext* context, const myproject::ReadRequest* request, myproject::ReadResponse* response) override;
    grpc::Status Update(grpc::ServerContext* context, const myproject::UpdateRequest* request, myproject::UpdateResponse* response) override;
    grpc::Status Delete(grpc::ServerContext* context, const myproject::DeleteRequest* request, myproject::DeleteResponse* response) override;

private:
	mysqlx::Session& DBlink; // 数据库链接
};

#endif // DATA_SERVICE_H