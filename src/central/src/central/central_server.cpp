#include "central_server.h"

CentralServerImpl::CentralServerImpl()
{
    try {
        std::cerr << "尝试连接到 Redis 服务器..." << std::endl;
        redis_client.get_client()->connect("127.0.0.1", 6379);
        std::cerr << "成功连接到 Redis 服务器" << std::endl;
    }
    catch (const cpp_redis::redis_error& e) {
        std::cerr << "Redis 连接错误: " << e.what() << std::endl;
        std::abort();
    }
    catch (const std::exception& e) {
        std::cerr << "其他异常: " << e.what() << std::endl;
        std::abort();
    }
    catch (...) {
        std::cerr << "未知异常" << std::endl;
        std::abort();
    }
}


CentralServerImpl::~CentralServerImpl()
{
	redis_client.get_client()->disconnect(); // 断开 redis连接
}

// 服务器注册
grpc::Status CentralServerImpl::RegisterServer(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response)
{
    std::string server_name = request->server_name();
    std::string address = request->address();
    int port = request->port();

    try 
    {
		// 数据以 server_name 为 key，address:port 为 value 存入 Redis
        redis_client.get_client()->hset("servers", server_name, address + ":" + std::to_string(port));
        redis_client.get_client()->sync_commit();
    }
    catch (const cpp_redis::redis_error& e) 
    {
        std::cerr << "Redis hset error: " << e.what() << std::endl;
        response->set_success(false);
        response->set_message("服务器注册失败");
        return grpc::Status::OK;
    }

	std::cout << "服务器注册成功: " << server_name << " " << address << " " << port << std::endl;
    response->set_success(true);
    response->set_message("服务器注册成功");
    return grpc::Status::OK;
}

// 断开服务器连接
grpc::Status CentralServerImpl::UnregisterServer(grpc::ServerContext* context, const myproject::UnregisterServerRequest* request, myproject::UnregisterServerResponse* response)
{
    std::string server_name = request->server_name();

    try {
        redis_client.get_client()->hdel("servers", { server_name });
        redis_client.get_client()->sync_commit();
    }
    catch (const cpp_redis::redis_error& e) {
        std::cerr << "Redis hdel error: " << e.what() << std::endl;
        response->set_success(false);
        response->set_message("服务器断开连接失败");
        return grpc::Status::OK;
    }

	std::cout << "服务器断开连接成功: " << server_name << std::endl;
    response->set_success(true);
    response->set_message("服务器断开连接成功");
    return grpc::Status::OK;
}

// 请求目标服务器信息
grpc::Status CentralServerImpl::GetServerInfo(grpc::ServerContext* context, const myproject::ServerInfoRequest* request, myproject::ServerInfoResponse* response)
{
    std::string server_name = request->server_name();

    // 从 Redis 获取服务器信息
	auto reply = redis_client.get_client()->hget("servers", server_name);   // 异步获取数据
	redis_client.get_client()->sync_commit();   // 提交请求
	auto future = reply.get();  // 获取异步结果

    if (future.is_null()) {
        response->set_success(false);
        response->set_message("服务器未注册");
    }
    else {
        std::string server_info = future.as_string();
        size_t pos = server_info.find(':');
        std::string address = server_info.substr(0, pos);
        int port = std::stoi(server_info.substr(pos + 1));

        response->set_success(true);
        response->set_address(address);
        response->set_port(port);
        response->set_message("服务器信息获取成功");
    }
    return grpc::Status::OK;
}

