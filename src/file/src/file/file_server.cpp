#include "file_server.h"

FileServerImpl::FileServerImpl(LoggerManager& logger_manager_):
    server_type(rpc_server::ServerType::FILE),    // 服务器类型
    logger_manager(logger_manager_),  // 日志管理器   
    central_stub(rpc_server::CentralServer::NewStub(grpc::CreateChannel("localhost:50050", grpc::InsecureChannelCredentials()))) // 中心服务器存根
{

    this->Read_server_config();   // 读取配置文件并初始化服务器地址和端口

    this->register_server();  // 注册服务器

    // 启动定时任务
    // 定时向中心服务器发送心跳包
    std::thread(&FileServerImpl::Send_heartbeat, this).detach();
}

FileServerImpl::~FileServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    unregister_server(); // 注销服务器

    // 记录关闭日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("FileServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

/*************************************** 多线程工具函数 *****************************************************************/
// 启动线程池
void FileServerImpl::start_thread_pool(int num_threads)
{// 相关注释请参考 /src/central/src/central/central_server.cpp/start_thread_pool()
    for(int i = 0; i < num_threads; ++i)
    {
        this->thread_pool.emplace_back(&FileServerImpl::Worker_thread, this);   // 创建线程
    }
}

// 停止线程池
void FileServerImpl::stop_thread_pool()
{// 相关注释请参考 /src/central/src/central/central_server.cpp/stop_thread_pool()
    {
        std::lock_guard<std::mutex> lock(this->queue_mutex);
        this->stop_threads = true;
    }

    this->queue_cv.notify_all();

    for(auto& thread : this->thread_pool)
    {
        if(thread.joinable())
        {
            thread.join();
        }
    }
    this->thread_pool.clear();

    std::queue<std::function<void()>> empty;
    std::swap(this->task_queue, empty);
}

// 添加异步任务
std::future<void> FileServerImpl::add_async_task(std::function<void()> task)
{
    auto task_ptr = std::make_shared<std::packaged_task<void()>>(std::move(task));
    std::future<void> task_future = task_ptr->get_future();

    {
        std::lock_guard<std::mutex> lock(this->queue_mutex);
        this->task_queue.push([task_ptr]() {
            (*task_ptr)();
        });
    }
    this->queue_cv.notify_one();
    return task_future;
}

// 线程池工作函数
void FileServerImpl::Worker_thread()
{// 相关注释请参考 /src/central/src/central/central_server.cpp/worker_thread()
    while(true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);

            this->queue_cv.wait(lock, [this] {
                return !this->task_queue.empty() || this->stop_threads;
            });

            if(this->stop_threads && this->task_queue.empty())
            {
                return;
            }
            task = std::move(this->task_queue.front());
            this->task_queue.pop();
        }
        task();
    }
}

/*************************************** 连接池管理 *********************************************************/
// 服务器注册
void FileServerImpl::register_server()
{
    // 请求
    rpc_server::RegisterServerReq req;
    req.set_server_type(rpc_server::ServerType::FILE);
    req.set_address(this->server_address);
    req.set_port(this->server_port);
    // 响应
    rpc_server::RegisterServerRes res;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = this->central_stub->Register_server(&context, req, &res);

    if(status.ok() && res.success())
    {
        this->logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("File server registered successfully: {} {}", this->server_address, this->server_port);
    }
    else
    {
        this->logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->error("File server registration failed: {} {}", this->server_address, this->server_port);
    }
}

// 注销服务器
void FileServerImpl::unregister_server()
{
    // 客户端
    grpc::ClientContext context;
    // 请求
    rpc_server::UnregisterServerReq req;
    req.set_server_type(rpc_server::ServerType::FILE);
    req.set_address(this->server_address);
    req.set_port(this->server_port);

    // 响应
    rpc_server::UnregisterServerRes res;

    grpc::Status status = this->central_stub->Unregister_server(&context, req, &res);

    if(status.ok() && res.success())
    {
        this->logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("File server unregistered successfully: {} {}", this->server_address, this->server_port);
    }
    else
    {
        this->logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->error("File server unregistration failed: {} {}", this->server_address, this->server_port);
    }
}

/******************************************* 定时任务 *****************************************************/
// 定时任务：发送心跳包
void FileServerImpl::Send_heartbeat()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒发送一次心跳包

        rpc_server::HeartbeatReq request;
        rpc_server::HeartbeatRes response;
        grpc::ClientContext context;

        request.set_server_type(rpc_server::ServerType::GATEWAY);
        request.set_address(this->server_address); // 设置服务器ip
        request.set_port(this->server_port); // 设置服务器端口

        grpc::Status status = central_stub->Heartbeat(&context, request, &response);

        if(status.ok() && response.success())
        {
            logger_manager.getLogger(LogCategory::HEARTBEAT)->info("Heartbeat sent successfully.");
        } else
        {
            logger_manager.getLogger(LogCategory::HEARTBEAT)->error("Failed to send heartbeat.");
        }
    }
}

/**************************************** grpc服务接口定义 **************************************************************************/
// 服务转发接口
grpc::Status FileServerImpl::Upload(grpc::ServerContext* context, const rpc_server::UploadReq* req, rpc_server::UploadRes* res)
{
    auto task_future = this->add_async_task([this, req, res] {
        // this->Handle_create(req, res);   // 执行函数
    });

    // 等待任务完成
    task_future.get();

    return grpc::Status::OK;
}

// 文件下载服务
grpc::Status FileServerImpl::Download(grpc::ServerContext* context, const rpc_server::DownloadReq* req, rpc_server::DownloadRes* res)
{
    return grpc::Status::OK;
}

// 文件删除服务
grpc::Status FileServerImpl::Delete(grpc::ServerContext* context, const rpc_server::DeleteFileReq* req, rpc_server::DeleteFileRes* res)
{
    return grpc::Status::OK;
}

// 文件列表服务
grpc::Status FileServerImpl::ListFiles(grpc::ServerContext* context, const rpc_server::ListFilesReq* req, rpc_server::ListFilesRes* res)
{
    return grpc::Status::OK;
}

/**************************************** grpc服务接口工具函数 **************************************************************************/

/******************************************** 其他工具函数 ***********************************************/
// 读取服务器配置文件，初始化服务器地址和端口
void FileServerImpl::Read_server_config()
{
    lua_State* L = luaL_newstate();  // 创建lua虚拟机
    luaL_openlibs(L);   // 打开lua标准库

    std::string file_url = "config/file_server_config.lua";  // 配置文件路径

    if(luaL_dofile(L, file_url.c_str()) != LUA_OK)
    {
        lua_close(L);
        throw std::runtime_error("Failed to load config file");
    }

    lua_getglobal(L, "file_server");
    if(!lua_istable(L, -1))
    {
        lua_close(L);
        throw std::runtime_error("Invalid config format");
    }

    lua_getfield(L, -1, "host");
    if(!lua_isstring(L, -1))
    {
        lua_close(L);
        throw std::runtime_error("Invalid host format");
    }
    this->server_address = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "port");
    if(!lua_isinteger(L, -1))
    {
        lua_close(L);
        throw std::runtime_error("Invalid port format");
    }
    this->server_port = std::to_string(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_close(L);   // 关闭lua虚拟机
}
