#ifndef DATA_SERVICE_H
#define DATA_SERVICE_H

#include "server_data.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <mysqlx/xdevapi.h> // mysql

// ���ݿ����ʵ����
class DatabaseServerImpl final : public myproject::DatabaseServer::Service {
public:
	explicit DatabaseServerImpl(mysqlx::Session& DBlink_); // ���캯�����������ݿ�����
    ~DatabaseServerImpl() override; // ���������������

	// ����ʵ��
	// ������ݿ��¼
    grpc::Status Create(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response) override;
	// ��ȡ���ݿ��¼
    grpc::Status Read(grpc::ServerContext* context, const myproject::ReadRequest* request, myproject::ReadResponse* response) override;
	// �������ݿ��¼
	grpc::Status Update(grpc::ServerContext* context, const myproject::UpdateRequest* request, myproject::UpdateResponse* response) override;
	// ɾ�����ݿ��¼
	grpc::Status Delete(grpc::ServerContext* context, const myproject::DeleteRequest* request, myproject::DeleteResponse* response) override;

private:
	mysqlx::Session& DBlink; // ���ݿ�����
};

#endif // DATA_SERVICE_H