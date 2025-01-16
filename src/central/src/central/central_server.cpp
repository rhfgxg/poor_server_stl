#include "central_server.h"

CentralServerImpl::CentralServerImpl(LoggerManager& logger_manager_, const std::string address, const std::string port):
    server_type(rpc_server::ServerType::CENTRAL),    // 服务器类型
    server_address(address),
    server_port(port),
    logger_manager(logger_manager_),    // 日志管理器
    central_connection_pool(10),    // 初始化中心服务器连接池大小为 10
    data_connection_pool(10),
    gateway_connection_pool(10),
    logic_connection_pool(10),
    login_connection_pool(10),
    file_connection_pool(10)
{
    // 连接池日志
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Central_connection_pool: initialized successfully, pool size: 10");
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Data_connection_pool: initialized successfully, pool size: 10");
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Gateway_connection_pool: initialized successfully, pool size: 10");
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("logic_connection_pool: initialized successfully, pool size: 10");
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("login_connection_pool: initialized successfully, pool size: 10");
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("file_connection_pool: initialized successfully, pool size: 10");

    // 将本服务器加入中心数据库链接池
    central_connection_pool.add_server(rpc_server::ServerType::CENTRAL, this->server_address,this->server_port);
    logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Central_connection_pool: successfully registered a server: {} {}", this->server_address, this->server_port);

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
        this->thread_pool.emplace_back(&CentralServerImpl::Worker_thread, this);   // 创建线程
    }
}

// 停止线程池
void CentralServerImpl::stop_thread_pool()
{
    {
        std::lock_guard<std::mutex> lock(this->queue_mutex);  // 加锁
        this->stop_threads = true;    // 线程停止标志设置为 true，通知它应该停止工作
    }

    this->queue_cv.notify_all();  // 通知所有线程，检查 stop_threads 标志

    for(auto& thread : this->thread_pool) // 遍历线程池
    {
        if(thread.joinable())   // 如果线程可加入
        {
            thread.join();  // 加入线程
        }
    }
    this->thread_pool.clear();  // 清空线程池
    // 清空任务队列
    std::queue<std::function<void()>> empty;
    std::swap(this->task_queue,empty);
}

// 添加异步任务
std::future<void> CentralServerImpl::add_async_task(std::function<void()> task)
{
    auto task_ptr = std::make_shared<std::packaged_task<void()>>(std::move(task));  // 创建任务
    std::future<void> task_future = task_ptr->get_future();    // 获取任务的 future 对象

    {
        std::lock_guard<std::mutex> lock(this->queue_mutex);  // 加锁
        this->task_queue.push([task_ptr]() {  // 将任务加入任务队列
            (*task_ptr)();  
        });
    }
    this->queue_cv.notify_one();    // 通知线程池有新任务

    return task_future;
}

// 线程池工作函数
void CentralServerImpl::Worker_thread()
{
    while(true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);  // 使用 unique_lock 加锁

            queue_cv.wait(lock, [this] {
                return !this->task_queue.empty() || (this->stop_threads);
            });  // 等待条件变量，直到任务队列不为空或停止标志为 true

            if(this->stop_threads && this->task_queue.empty())  // 如果停止标志为 true 且任务队列为空，退出线程
            {
                return;
            }

            task = std::move(this->task_queue.front());  // 取出任务队列中的第一个任务
            this->task_queue.pop();  // 移除任务队列中的第一个任务
        }
        task();  // 执行任务
    }
}

/**************************************** grpc服务接口 ************************************************/
// 服务器注册
grpc::Status CentralServerImpl::Register_server(grpc::ServerContext* context, const rpc_server::RegisterServerReq* req, rpc_server::RegisterServerRes* res)
{/* 函数解释
  * 参数列表：发起请求的客户端信息context，服务请求request，服务响应response
  * 返回值：服务执行状态（成功，异常）
  * 请求与响应定义在 服务器对应的 proto文件中
  */

    auto task_future = this->add_async_task([this, req, res] {
        this->Handle_register_server(req, res); // 执行注册服务器
    });

    // 等待任务完成
    task_future.get();

    return grpc::Status::OK;
}

// 服务器断开
grpc::Status CentralServerImpl::Unregister_server(grpc::ServerContext* context, const rpc_server::UnregisterServerReq* req, rpc_server::UnregisterServerRes* res)
{
    auto task_future = this->add_async_task([this,req,res] {
        rpc_server::ServerType server_type = req->server_type(); // 服务器类型  
        std::string address = req->address();   // 服务器地址
        std::string port = req->port(); // 服务器端口

        this->Release_server_connection(server_type, address, port);

        res->set_success(true);
        res->set_message("successfully unregistered");
    });

    // 等待任务完成
    task_future.get();

    return grpc::Status::OK;
}

// 获取连接池中所有链接
grpc::Status CentralServerImpl::Get_connec_poor(grpc::ServerContext* context, const rpc_server::MultipleConnectPoorReq* req, rpc_server::MultipleConnectPoorRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        this->All_connec_poor(req, res);
    });

    task_future.get();
    return grpc::Status::OK;
}

// 接收心跳包
grpc::Status CentralServerImpl::Heartbeat(grpc::ServerContext* context, const rpc_server::HeartbeatReq* req, rpc_server::HeartbeatRes* res)
{
    std::lock_guard<std::mutex> lock(this->heartbeat_mutex);  // 加锁
    this->heartbeat_records[req->address()] = {req->server_type(),req->address(),req->port(), std::chrono::steady_clock::now()}; // 记录心跳时间
    res->set_success(true);    // 设置响应成功
    res->set_message("Heartbeat received");
    return grpc::Status::OK;
}

/************************************* grpc服务接口工具函数 **********************************************/
// 执行注册服务器
void CentralServerImpl::Handle_register_server(const rpc_server::RegisterServerReq* req, rpc_server::RegisterServerRes* res)
{
    rpc_server::ServerType server_type = req->server_type(); // 服务器类型
    std::string address = req->address();   // 服务器地址
    std::string port = req->port(); // 服务器端口

    // 根据服务器类型，将服务器加入连接池
    switch(server_type)
    {
    case rpc_server::ServerType::CENTRAL:    // 中心服务器
    {
        // 将链接加入连接池并记录日志
        this->central_connection_pool.add_server(rpc_server::ServerType::CENTRAL, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Central_connection_pool: successfully registered a server: {} {}", address, port);
        res->set_success(true);
        res->set_message("Server registered successfully");
        break;
    }
    case rpc_server::ServerType::DATA:   // 数据库服务器
    {
        this->data_connection_pool.add_server(rpc_server::ServerType::DATA, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Data_connection_pool: successfully registered a server: {} {}", address, port);
        res->set_success(true);
        res->set_message("Server registered successfully");
        break;
    }
    case rpc_server::ServerType::GATEWAY:    // 网关服务器
    {
        this->gateway_connection_pool.add_server(rpc_server::ServerType::GATEWAY, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Gateway_connection_pool: successfully registered a server: {} {}", address, port);
        res->set_success(true);
        res->set_message("Server registered successfully");
        break;
    }
    case rpc_server::ServerType::LOGIC:      // 逻辑服务器
    {
        this->logic_connection_pool.add_server(rpc_server::ServerType::LOGIC, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Logic_connection_pool: successfully registered a server: {} {}", address, port);
        res->set_success(true);
        res->set_message("Server registered successfully");
        break;
    }
    case rpc_server::ServerType::LOGIN:      // 登录服务器
    {
        this->login_connection_pool.add_server(rpc_server::ServerType::LOGIN, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("Login_connection_pool: successfully registered a server: {} {}", address, port);
        res->set_success(true);
        res->set_message("Server registered successfully");
        break;
    }
    case rpc_server::ServerType::FILE:      // 文件服务器
    {
        this->file_connection_pool.add_server(rpc_server::ServerType::FILE, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->info("File_connection_pool: successfully registered a server: {} {}", address, port);
        res->set_success(true);
        res->set_message("Server registered successfully");
        break;
    }
    default:
    {
        res->set_success(false);
        res->set_message("Invalid server type");
    }
    }
}

// 释放连接池中服务器连接
void CentralServerImpl::Release_server_connection(rpc_server::ServerType server_type, const std::string& address,const std::string& port)
{// 被 Unregister_server() 和 check_heartbeat() 调用
    // 根据服务器类型，从连接池中删除服务器
    switch(server_type)
    {
    case rpc_server::ServerType::CENTRAL:    // 中心服务器
    {
        // 连接池中移除链接，并记录日志
        this->central_connection_pool.remove_server(rpc_server::ServerType::CENTRAL, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->warn("Central_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    case rpc_server::ServerType::DATA:   // 数据库服务器
    {
        this->data_connection_pool.remove_server(rpc_server::ServerType::DATA, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->warn("Data_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    case rpc_server::ServerType::GATEWAY:    // 网关服务器
    {
        this->gateway_connection_pool.remove_server(rpc_server::ServerType::GATEWAY, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->warn("Gateway_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    case rpc_server::ServerType::LOGIC:      // 逻辑服务器
    {
        this->logic_connection_pool.remove_server(rpc_server::ServerType::LOGIC, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->warn("Logic_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    case rpc_server::ServerType::LOGIN:      // 登录服务器
    {
        this->login_connection_pool.remove_server(rpc_server::ServerType::LOGIN, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->warn("Login_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    case rpc_server::ServerType::FILE:      // 文件服务器
    {
        this->file_connection_pool.remove_server(rpc_server::ServerType::FILE, address, port);
        this->logger_manager.getLogger(LogCategory::CONNECTION_POOL)->warn("File_connection_pool: successfully unregistered a server: {} {}", address, port);
        break;
    }
    }
}

// 获取链接池中的所有链接
void CentralServerImpl::All_connec_poor(const rpc_server::MultipleConnectPoorReq* req, rpc_server::MultipleConnectPoorRes* res)
{
    for(const auto& server_type : req->server_types())
    {
        rpc_server::ConnectPool* connect_pool = res->add_connect_pools();
        connect_pool->set_server_type(static_cast<rpc_server::ServerType>(server_type));

        std::unordered_map<rpc_server::ServerType, std::vector<std::pair<std::string, std::string>>> connections;

        switch(server_type)
        {
        case rpc_server::CENTRAL:
        {
            connections = central_connection_pool.get_all_connections();
            break;
        }
        case rpc_server::DATA:
        {
            connections = data_connection_pool.get_all_connections();
            break;
        }
        case rpc_server::GATEWAY:
        {
            connections = gateway_connection_pool.get_all_connections();
            break;
        }
        case rpc_server::LOGIC:
        {
            connections = logic_connection_pool.get_all_connections();
            break;
        }
        case rpc_server::LOGIN:
        {
            connections = login_connection_pool.get_all_connections();
            break;
        }
        case rpc_server::FILE:
        {
            connections = file_connection_pool.get_all_connections();
            break;
        }
        default:
        {
            res->set_success(false);
            res->set_message("Invalid server type");
            return;
        }
        }

        for(const auto& connection : connections[static_cast<rpc_server::ServerType>(server_type)])
        {
            rpc_server::ConnectInfo* conn_info = connect_pool->add_connect_info();
            conn_info->set_address(connection.first);
            conn_info->set_port(std::stoi(connection.second));
        }
    }

    res->set_success(true);
    res->set_message("Connection pools information retrieved successfully");
}

/******************************************* 定时任务 **************************************************/
// 定时检查心跳记录
void CentralServerImpl::check_heartbeat()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒检查一次心跳记录
        std::lock_guard<std::mutex> lock(this->heartbeat_mutex);  // 加锁

        auto now = std::chrono::steady_clock::now();    // 当前时间
        for(const auto& record : this->heartbeat_records) // 遍历心跳记录
        {
            if(std::chrono::duration_cast<std::chrono::seconds>(now - record.second.last_heartbeat).count() > 30)  
            {// 如果心跳时间超过30秒，释放服务器连接
                this->logger_manager.getLogger(LogCategory::HEARTBEAT)->warn("Server lost heartbeat, address {} port ()", record.second.address, record.second.port);
                this->Release_server_connection(record.second.server_type, record.second.address, record.second.port);
            }
        }
    }
}

/******************************************** 其他工具函数 ***********************************************/
