#include "central_server.h"

// 服务器注册
grpc::Status register_server(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response)
{
	// 将注册信息写入配置文件或缓存，方便后续读取

	response->set_success(true);
	response->set_message("服务器注册成功");
}

// 请求目标服务器信息
grpc::Status get_server_info(grpc::ServerContext* context, const myproject::CreateRequest* request, myproject::CreateResponse* response)
{
	// 根据负载均衡等功能，得到合适的服务器信息

	response->set_success(true);
	response->set_message("获得目标服务器信息");
}