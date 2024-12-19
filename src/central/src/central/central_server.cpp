#include "central_server.h"

CentralServerImpl::CentralServerImpl(LoggerManager& logger_manager_) :
    logger_manager(logger_manager_),
    central_connection_pool(10),
    data_connection_pool(10),   // 数据库连接池大小为 10
    gateway_connection_pool(10),    // 网关连接池大小为 10
	logic_connection_pool(10),  // 逻辑服务器连接池大小为 10
    login_connection_pool(10)   // 登录连接池大小为 10
{
    // 在中心服务器连接池中加入本服务器
    central_connection_pool.add_server(myproject::ServerType::CENTRAL, "localhost", "50050");
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("中心服务器注册成功: {} {}", "localhost", "50050");

    //// 启动定时任务，定时检测服务器运行状态
    //std::thread([this]() {
    //    while (true) {
    //        // 检测服务器运行状态
    //        // 更新连接池
    //        std::this_thread::sleep_for(std::chrono::seconds(60));
    //    }
    //    }).detach();
}

CentralServerImpl::~CentralServerImpl()
{
}

// 服务器注册
grpc::Status CentralServerImpl::RegisterServer(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response)
{
    myproject::ServerType server_type = request->server_type(); // 服务器类型
    std::string address = request->address();   // 服务器地址
    std::string port = request->port(); // 服务器端口

    // 根据服务器类型，将服务器加入连接池
    switch (server_type)
    {
    case myproject::ServerType::CENTRAL:
    {
        // todo：中心服务器
        logger_manager.getLogger(LogCategory::REQUESTS_RESPONSES)->info("中心服务器注册成功: {} {}暂未实现", address, port);
        break;
    }
    case myproject::ServerType::DATA:   // 数据库服务器
    {
        data_connection_pool.add_server(myproject::ServerType::DATA, address, port);    // 加入连接池
        logger_manager.getLogger(LogCategory::REQUESTS_RESPONSES)->info("数据库服务器注册成功: {} {}", address, port);
        break;
    }
    case myproject::ServerType::GATEWAY:    // 网关服务器
    {
        gateway_connection_pool.add_server(myproject::ServerType::GATEWAY, address, port);
        logger_manager.getLogger(LogCategory::REQUESTS_RESPONSES)->info("网关服务器注册成功: {} {}", address, port);
        break;
    }
    case myproject::ServerType::LOGIN:      // 登录服务器
    {
        login_connection_pool.add_server(myproject::ServerType::LOGIN, address, port);
        logger_manager.getLogger(LogCategory::REQUESTS_RESPONSES)->info("登录服务器注册成功: {} {}", address, port);
        break;
    }
    }

    response->set_success(true);
    response->set_message("服务器注册成功");
    return grpc::Status::OK;
}

// 断开服务器连接
grpc::Status CentralServerImpl::UnregisterServer(grpc::ServerContext* context, const myproject::UnregisterServerRequest* request, myproject::UnregisterServerResponse* response)
{
    myproject::ServerType server_type = request->server_type(); // 服务器类型  
    std::string address = request->address();   // 服务器地址
    std::string port = request->port(); // 服务器端口

    // 根据服务器类型，从连接池中删除服务器
    switch (server_type)
    {
    case myproject::ServerType::CENTRAL:
    {
        // todo：中心服务器
        logger_manager.getLogger(LogCategory::REQUESTS_RESPONSES)->info("数据库服务器断开连接成功: {} {}暂未实现", address, port);
        break;
    }
    case myproject::ServerType::DATA:   // 数据库服务器
    {
        data_connection_pool.remove_server(myproject::ServerType::DATA, address, port);    // 从连接池中删除
        logger_manager.getLogger(LogCategory::REQUESTS_RESPONSES)->info("数据库服务器断开连接成功: {} {}", address, port);
        break;
    }
    case myproject::ServerType::GATEWAY:    // 网关服务器
    {
        gateway_connection_pool.remove_server(myproject::ServerType::GATEWAY, address, port);
        logger_manager.getLogger(LogCategory::REQUESTS_RESPONSES)->info("网关服务器断开连接成功: {} {}", address, port);
        break;
    }
    case myproject::ServerType::LOGIN:      // 登录服务器
    {
        login_connection_pool.remove_server(myproject::ServerType::LOGIN, address, port);
        logger_manager.getLogger(LogCategory::REQUESTS_RESPONSES)->info("登录服务器断开连接成功: {} {}", address, port);
        break;
    }
    default:
        logger_manager.getLogger(LogCategory::REQUESTS_RESPONSES)->info("尝试断开未知服务器: {} {}", address, port);
        response->set_success(false);
        response->set_message("无效的服务器类型");
        return grpc::Status::OK;
    }
}

// 获取连接池中所有链接
grpc::Status CentralServerImpl::GetConnectPoor(grpc::ServerContext* context, const myproject::ConnectPoorRequest* request, myproject::ConnectPoorResponse* response)
{
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("获取连接池信息");
    myproject::ServerType server_type = request->server_type(); // 服务器类型  

    std::unordered_map<myproject::ServerType, std::vector<std::pair<std::string, std::string>>> connections;

    switch (server_type)
    {
    case myproject::CENTRAL:
    {
        // 获取中央服务器连接池信息
        connections = central_connection_pool.get_all_connections();
        break;
    }
    case myproject::DATA:
    {
        // 获取数据库服务器连接池信息
        connections = data_connection_pool.get_all_connections();
        break;
    }
    case myproject::GATEWAY:
    {
        // 获取网关服务器连接池信息
        connections = gateway_connection_pool.get_all_connections();
        break;
    }
    case myproject::LOGIN:
    {
        // 获取登录服务器连接池信息
        connections = login_connection_pool.get_all_connections();
        break;
    }
    default:
        response->set_success(false);
        response->set_message("无效的服务器类型");
        return grpc::Status::OK;
    }

    for (const auto& connection : connections[server_type])
    {
        auto* conn_info = response->add_connect_info();
        conn_info->set_address(connection.first);
        conn_info->set_port(std::stoi(connection.second)); // 将端口从字符串转换为整数
    }

    response->set_success(true);
    response->set_message("获取连接池信息成功");
    return grpc::Status::OK;
}