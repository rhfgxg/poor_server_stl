#include "central_server.h"

CentralServerImpl::CentralServerImpl(LoggerManager& logger_manager_):
    logger_manager(logger_manager_),    // 日志管理器
    central_connection_pool(10),    // 初始化中心服务器连接池大小为 10
    data_connection_pool(10),
    gateway_connection_pool(10),
    logic_connection_pool(10),
    login_connection_pool(10)
{
    // 连接池日志
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Central_connection_pool: initialized successfully, pool size: 10");
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Data_connection_pool: initialized successfully, pool size: 10");
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Gateway_connection_pool: initialized successfully, pool size: 10");
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("logic_connection_pool: initialized successfully, pool size: 10");
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("login_connection_pool: initialized successfully, pool size: 10");

    // 在中心服务器连接池中加入本服务器
    central_connection_pool.add_server(myproject::ServerType::CENTRAL,"localhost","50050");
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Central_connection_pool: successfully registered a server: {} {}", "localhost", "50050");

    // 启动定时任务，定时检测心跳包
    std::thread(&CentralServerImpl::check_heartbeat, this).detach();
}

CentralServerImpl::~CentralServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    // 记录关闭日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("Central_server stopped");

    logger_manager.cleanup();   // 停止并清理日志管理器
    // 自动清理连接池
}

/******************************************* 线程池 **************************************************/
// 启动线程池
void CentralServerImpl::start_thread_pool(int num_threads)
{
    for(int i=0; i<num_threads; ++i)
    {
        thread_pool.emplace_back(&CentralServerImpl::worker_thread, this);   // 创建线程
    }
}

// 停止线程池
void CentralServerImpl::stop_thread_pool()
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        stop_threads = true;    // 线程停止标志设置为 true，通知它应该停止工作
    }

    queue_cv.notify_all();  // 通知所有线程，检查 stop_threads 标志

    for(auto& thread : thread_pool) // 遍历线程池
    {
        if(thread.joinable())   // 如果线程可加入
        {
            thread.join();  // 加入线程
        }
    }
    thread_pool.clear();  // 清空线程池
    // 清空任务队列
    std::queue<std::function<void()>> empty;
    std::swap(task_queue,empty);
}

// 线程池工作函数
void CentralServerImpl::worker_thread()
{
    while(true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);  // 使用 unique_lock 加锁
            queue_cv.wait(lock,[this] {
                return !task_queue.empty() || stop_threads;
            });  // 等待条件变量，直到任务队列不为空或停止标志为 true

            if(stop_threads && task_queue.empty())  // 如果停止标志为 true 且任务队列为空，退出线程
            {
                return;
            }
            task = std::move(task_queue.front());  // 取出任务队列中的第一个任务
            task_queue.pop();  // 移除任务队列中的第一个任务
        }
        task();  // 执行任务
    }
}

/**************************************** grpc服务接口 ************************************************/
// 服务器注册
grpc::Status CentralServerImpl::RegisterServer(grpc::ServerContext* context, const myproject::RegisterServerRequest* request, myproject::RegisterServerResponse* response)
{/* 函数解释
  * 参数列表：发起请求的客户端信息context，服务请求request，服务响应response
  * 返回值：服务执行状态（成功，异常）
  * 请求与响应定义在 服务器对应的 proto文件中
  */
    myproject::ServerType server_type = request->server_type(); // 服务器类型
    std::string address = request->address();   // 服务器地址
    std::string port = request->port(); // 服务器端口

    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this,server_type, address, port] {   // lambda函数 将任务加入任务队列
            // 根据服务器类型，将服务器加入连接池
            switch(server_type)
            {
            case myproject::ServerType::CENTRAL:    // 中心服务器
            {
                // 将链接加入连接池并记录日志
                data_connection_pool.add_server(myproject::ServerType::CENTRAL, address, port);
                logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Central_connection_pool: successfully registered a server: {} {}", address, port);
                break;
            }
            case myproject::ServerType::DATA:   // 数据库服务器
            {
                data_connection_pool.add_server(myproject::ServerType::DATA, address, port);
                logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Data_connection_pool: successfully registered a server: {} {}", address, port);
                break;
            }
            case myproject::ServerType::GATEWAY:    // 网关服务器
            {
                gateway_connection_pool.add_server(myproject::ServerType::GATEWAY, address, port);
                logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Gateway_connection_pool: successfully registered a server: {} {}", address, port);
                break;
            }
            case myproject::ServerType::LOGIC:      // 逻辑服务器
            {
                login_connection_pool.add_server(myproject::ServerType::LOGIC, address, port);
                logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Logic_connection_pool: successfully registered a server: {} {}", address, port);
                break;
            }
            case myproject::ServerType::LOGIN:      // 登录服务器
            {
                login_connection_pool.add_server(myproject::ServerType::LOGIN, address, port);
                logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Login_connection_pool: successfully registered a server: {} {}", address, port);
                break;
            }
            }
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务

    response->set_success(true);
    response->set_message("服务器注册成功");
    return grpc::Status::OK;
}

// 服务器断开
grpc::Status CentralServerImpl::UnregisterServer(grpc::ServerContext* context, const myproject::UnregisterServerRequest* request, myproject::UnregisterServerResponse* response)
{
    myproject::ServerType server_type = request->server_type(); // 服务器类型  
    std::string address = request->address();   // 服务器地址
    std::string port = request->port(); // 服务器端口

    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this, server_type, address, port] { 
            // 根据服务器类型，从连接池中删除服务器
            release_server_connection(server_type, address, port);
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务

    response->set_success(true);
    response->set_message("successfully unregistered");
    return grpc::Status::OK;
}

// 获取连接池中所有链接
grpc::Status CentralServerImpl::GetConnectPoor(grpc::ServerContext* context, const myproject::ConnectPoorRequest* request, myproject::ConnectPoorResponse* response)
{
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Get connection pool information");
    myproject::ServerType server_type = request->server_type(); // 服务器类型  

    std::unordered_map<myproject::ServerType, std::vector<std::pair<std::string, std::string>>> connections;

    switch(server_type)
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
    response->set_message("Invalid server type");
    return grpc::Status::OK;
    }

    for(const auto& connection : connections[server_type])
    {
        auto* conn_info = response->add_connect_info();
        conn_info->set_address(connection.first);
        conn_info->set_port(std::stoi(connection.second)); // 将端口从字符串转换为整数
    }

    response->set_success(true);
    response->set_message("Connection pool information retrieved successfully");
    return grpc::Status::OK;
}

// 接收心跳包
grpc::Status CentralServerImpl::Heartbeat(grpc::ServerContext* context, const myproject::HeartbeatRequest* request, myproject::HeartbeatResponse* response)
{
    std::lock_guard<std::mutex> lock(heartbeat_mutex);  // 加锁
    heartbeat_records[request->address()] = {request->server_type(), request->address(), request->port(), std::chrono::steady_clock::now()}; // 记录心跳时间
    response->set_success(true);    // 设置响应成功
    response->set_message("Heartbeat received");
    return grpc::Status::OK;
}

/************************************* grpc服务接口工具函数 **********************************************/
// 释放连接池中服务器连接
void CentralServerImpl::release_server_connection(myproject::ServerType server_type, const std::string& address,const std::string& port)
{
    // 根据服务器类型，从连接池中删除服务器
    switch(server_type)
    {
    case myproject::ServerType::CENTRAL:    // 中心服务器
    {
        // 连接池中移除链接，并记录日志
        login_connection_pool.remove_server(myproject::ServerType::CENTRAL, address, port);
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Central_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    case myproject::ServerType::DATA:   // 数据库服务器
    {
        data_connection_pool.remove_server(myproject::ServerType::DATA, address, port);
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Data_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    case myproject::ServerType::GATEWAY:    // 网关服务器
    {
        gateway_connection_pool.remove_server(myproject::ServerType::GATEWAY, address, port);
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Gateway_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    case myproject::ServerType::LOGIC:      // 逻辑服务器
    {
        login_connection_pool.remove_server(myproject::ServerType::LOGIC, address, port);
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Logic_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    case myproject::ServerType::LOGIN:      // 登录服务器
    {
        login_connection_pool.remove_server(myproject::ServerType::LOGIN, address, port);
        logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Login_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    }
}

/******************************************* 定时任务 **************************************************/
// 定时检查心跳记录
void CentralServerImpl::check_heartbeat()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒检查一次心跳记录
        std::lock_guard<std::mutex> lock(heartbeat_mutex);  // 加锁

        auto now = std::chrono::steady_clock::now();    // 当前时间
        for(const auto& record : heartbeat_records) // 遍历心跳记录
        {
            if(std::chrono::duration_cast<std::chrono::seconds>(now - record.second.last_heartbeat).count() > 30)  
            {// 如果心跳时间超过30秒，释放服务器连接
                std::cout << "Server " << record.first << " is offline." << std::endl;
                release_server_connection(record.second.server_type, record.second.address, record.second.port);
            }
        }
    }
}
