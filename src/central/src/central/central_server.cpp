#include "central_server.h"

// ������ע��
grpc::Status register_server(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response)
{
	// ��ע����Ϣд�������ļ��򻺴棬���������ȡ

	response->set_success(true);
	response->set_message("������ע��ɹ�");
}

// ����Ŀ���������Ϣ
grpc::Status get_server_info(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response)
{
	// ���ݸ��ؾ���ȹ��ܣ��õ����ʵķ�������Ϣ

	response->set_success(true);
	response->set_message("���Ŀ���������Ϣ");
}