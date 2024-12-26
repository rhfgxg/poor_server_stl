#include "data_server.h"
#include <codecvt>
#include <locale>

// 数据库服务实现
DatabaseServerImpl::DatabaseServerImpl(LoggerManager& logger_manager_,DBConnectionPool& db_pool_):
    logger_manager(logger_manager_),    // 日志管理器
    db_pool(db_pool_),    // 数据库连接池
    central_stub(myproject::CentralServer::NewStub(grpc::CreateChannel("localhost:50050",grpc::InsecureChannelCredentials()))) // 中心服务器存根
{
    register_server(); // 注册服务器
}

DatabaseServerImpl::~DatabaseServerImpl()
{
    stop_thread_pool(); // 停止并清空线程池

    unregister_server(); // 注销服务器

    // 记录关闭日志
    logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("DatabaseServer stopped");
    // 停止并清理日志管理器
    logger_manager.cleanup();
}

// 启动线程池
void DatabaseServerImpl::start_thread_pool(int num_threads)
{
    for(int i = 0; i < num_threads; ++i)
    {
        thread_pool.emplace_back(&DatabaseServerImpl::worker_thread,this);   // 创建线程
    }
}

// 停止线程池
void DatabaseServerImpl::stop_thread_pool()
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        stop_threads = true;    // 标志设置为 true，通知它应该停止工作
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
void DatabaseServerImpl::worker_thread()
{
    while(true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            queue_cv.wait(lock,[this] {
                return !task_queue.empty() || stop_threads;
            });

            if(stop_threads && task_queue.empty())
            {
                return;
            }
            task = std::move(task_queue.front());
            task_queue.pop();
        }
        task();
    }
}

// 注册服务器
void DatabaseServerImpl::register_server()
{
    // 请求
    myproject::RegisterServerRequest request;
    request.set_server_type(myproject::ServerType::DATA);
    request.set_address("127.0.0.1");
    request.set_port("50052");
    // 响应
    myproject::RegisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->RegisterServer(&context,request,&response);

    if(status.ok() && response.success())
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("数据库服务器注册成功: {} {}","localhost","50052");

    } else
    {
        logger_manager.getLogger(LogCategory::STARTUP_SHUTDOWN)->info("数据库服务器注册失败: {} {}","localhost","50052");
    }
}

// 注销服务器
void DatabaseServerImpl::unregister_server()
{
    // 请求
    myproject::UnregisterServerRequest request;
    request.set_server_type(myproject::ServerType::DATA);
    request.set_address("localhost");
    request.set_port("50052");

    // 响应
    myproject::UnregisterServerResponse response;

    // 客户端
    grpc::ClientContext context;

    grpc::Status status = central_stub->UnregisterServer(&context,request,&response);

    if(status.ok() && response.success())
    {
        std::cout << "服务器注销成功: " << response.message() << std::endl;
    } else
    {
        std::cerr << "服务器注销失败: " << response.message() << std::endl;
    }
}

// 添加
grpc::Status DatabaseServerImpl::Create(grpc::ServerContext* context,const myproject::CreateRequest* request,myproject::CreateResponse* response)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this,request,response] {
            handle_create(request,response);
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务
    return grpc::Status::OK;
}

// 查询
grpc::Status DatabaseServerImpl::Read(grpc::ServerContext* context,const myproject::ReadRequest* request,myproject::ReadResponse* response)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this,request,response] {
            handle_read(request,response);
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务
    return grpc::Status::OK;
}

// 更新
grpc::Status DatabaseServerImpl::Update(grpc::ServerContext* context,const myproject::UpdateRequest* request,myproject::UpdateResponse* response)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this,request,response] {
            handle_update(request,response);
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务
    return grpc::Status::OK;
}

// 删除
grpc::Status DatabaseServerImpl::Delete(grpc::ServerContext* context,const myproject::DeleteRequest* request,myproject::DeleteResponse* response)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);  // 加锁
        task_queue.push([this,request,response] {
            handle_delete(request,response);
        });
    }
    queue_cv.notify_one();  // 通知线程池有新任务
    return grpc::Status::OK;
}

// 处理数据库操作的函数
void DatabaseServerImpl::handle_create(const myproject::CreateRequest* request,myproject::CreateResponse* response)
{
    // 获取数据库连接
    mysqlx::Session session = db_pool.get_connection();

    // 获取请求参数
    std::string db_name = request->database();
    std::string tab_name = request->table();
    // 假设数据库操作
    // ...
    // 设置响应参数
    response->set_success(true);
    response->set_message("创建成功");
    std::cout << "数据库创建成功" << std::endl;

    // 释放数据库连接
    db_pool.release_connection(std::move(session));
}

void DatabaseServerImpl::handle_read(const myproject::ReadRequest* request,myproject::ReadResponse* response)
{
    try
    {
        // 获取数据库连接
        mysqlx::Session session = db_pool.get_connection();

        // 获取请求的 query 参数
        const std::string& database = request->database();
        const std::string& table = request->table();
        const auto& query = request->query();

        // 选择数据库
        mysqlx::Table tbl = session.getSchema(database).getTable(table);

        // 构建查询条件
        std::string condition;
        for(const auto& q : query)
        {
            if(!condition.empty())
            {
                condition += " AND ";
            }
            condition += q.first + " = '" + q.second + "'";
        }

        // 执行查询
        mysqlx::RowResult result = tbl.select("*").where(condition).execute();

        // 设置响应：查询结果
        for(mysqlx::Row row : result)
        {
            myproject::ReadResponse::Result* response_result = response->add_results();
            for(size_t i = 0; i < row.colCount(); ++i)
            {
                std::string column_name = result.getColumn(i).getColumnName();
                std::string column_value;
                if(row[i].isNull())
                {
                    column_value = "NULL";
                } else if(row[i].getType() == mysqlx::Value::Type::STRING) {
                    column_value = row[i].get<std::string>();
                } else if(row[i].getType() == mysqlx::Value::Type::VNULL) {
                    column_value = "VNULL";
                } else if(row[i].getType() == mysqlx::Value::Type::INT64) {
                    column_value = std::to_string(row[i].get<int64_t>());
                } else if(row[i].getType() == mysqlx::Value::Type::UINT64) {
                    column_value = std::to_string(row[i].get<uint64_t>());
                } else if(row[i].getType() == mysqlx::Value::Type::FLOAT) {
                    column_value = std::to_string(row[i].get<float>());
                } else if(row[i].getType() == mysqlx::Value::Type::DOUBLE) {
                    column_value = std::to_string(row[i].get<double>());
                } else if(row[i].getType() == mysqlx::Value::Type::BOOL) {
                    column_value = row[i].get<bool>() ? "true" : "false";
                } else if(row[i].getType() == mysqlx::Value::Type::RAW) {
                    column_value = "Raw Data"; // 需要进一步处理原始数据
                } else {
                    column_value = "Unsupported Type";
                }
                response_result->mutable_fields()->insert({column_name,column_value});
            }
        }

        response->set_success(true);
        response->set_message("查询成功");

        // 释放数据库连接
        db_pool.release_connection(std::move(session));
    } catch(const mysqlx::Error& err)
    {
        std::cerr << "数据库查询错误: " << err.what() << std::endl;
        response->set_success(false);
        response->set_message("数据库查询错误");
    } catch(std::exception& ex)
    {
        std::cerr << "异常: " << ex.what() << std::endl;
        response->set_success(false);
        response->set_message("异常");
    } catch(...)
    {
        std::cerr << "未知错误!" << std::endl;
        response->set_success(false);
        response->set_message("未知错误");
    }
}

void DatabaseServerImpl::handle_update(const myproject::UpdateRequest* request,myproject::UpdateResponse* response)
{
    // 获取数据库连接
    mysqlx::Session session = db_pool.get_connection();

    // 获取请求参数
    std::string db_name = request->database();
    std::string tab_name = request->table();
    // 假设数据库操作
    // ...
    // 设置响应参数
    response->set_success(true);
    response->set_message("更新成功");
    std::cout << "数据库更新成功" << std::endl;

    // 释放数据库连接
    db_pool.release_connection(std::move(session));
}

void DatabaseServerImpl::handle_delete(const myproject::DeleteRequest* request,myproject::DeleteResponse* response)
{
    // 获取数据库连接
    mysqlx::Session session = db_pool.get_connection();

    // 获取请求参数
    std::string db_name = request->database();
    std::string tab_name = request->table();
    // 假设数据库操作
    // ...
    // 设置响应参数
    response->set_success(true);
    response->set_message("删除成功");
    std::cout << "数据库删除成功" << std::endl;

    // 释放数据库连接
    db_pool.release_connection(std::move(session));
}

/***************************** 数据库链接池 **************************************************************************/

DBConnectionPool::DBConnectionPool(const std::string& uri,size_t pool_size):
    uri(uri),
    pool_size(pool_size)
{
    for(size_t i = 0; i < pool_size; ++i)
    {
        pool.push(mysqlx::Session(uri));    // 创建数据库连接，并加入连接池
    }
}

DBConnectionPool::~DBConnectionPool()
{
    while(!pool.empty())
    {
        pool.front().close();
        pool.pop();
    }
}

// 获取数据库连接
mysqlx::Session DBConnectionPool::get_connection()
{
    std::unique_lock<std::mutex> lock(pool_mutex);  // 加锁
    pool_cv.wait(lock,[this] {
        return !pool.empty();
    });    // 等待连接池不为空

    mysqlx::Session session = std::move(pool.front());  // 获取数据库连接
    pool.pop(); // 弹出连接
    return session;
}

// 释放数据库连接
void DBConnectionPool::release_connection(mysqlx::Session session)
{
    std::lock_guard<std::mutex> lock(pool_mutex);   // 加锁
    pool.push(std::move(session));  // 释放数据库连接
    pool_cv.notify_one();   // 通知有新的连接
}
