#include "logger_manager.h"

void LoggerManager::initialize(rpc_server::ServerType server_type)
{
    // 创建日志文件夹
    this->Create_log_directory(server_type);

    // 创建控制台日志器
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");

    // 创建不同分类的文件日志器
    auto create_file_logger = [&](poor::LogCategory category, const std::string& log_type) {
        auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_directory + "/" + log_type + "/log.log", 0, 0); // 按日期滚动
        file_sink->set_level(spdlog::level::debug);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");

        spdlog::sinks_init_list sink_list = {console_sink,file_sink};
        auto logger = std::make_shared<spdlog::logger>(std::to_string(static_cast<int>(category)), sink_list.begin(), sink_list.end());
        logger->set_level(spdlog::level::debug);
        this->loggers[category] = logger;
    };

    // 创建所有分类的日志器
    create_file_logger(poor::LogCategory::STARTUP_SHUTDOWN, "startup_shutdown");
    create_file_logger(poor::LogCategory::APPLICATION_ACTIVITY, "application_activity");
    create_file_logger(poor::LogCategory::CONNECTION_POOL, "connection_pool");
    create_file_logger(poor::LogCategory::SYSTEM_MONITORING, "system_monitoring");
    create_file_logger(poor::LogCategory::HEARTBEAT,"heartbeat");
    create_file_logger(poor::LogCategory::SECURITY, "security");
    create_file_logger(poor::LogCategory::CONFIGURATION_CHANGES, "configuration_changes");
    create_file_logger(poor::LogCategory::DATABASE_OPERATIONS,"database_operations");
    create_file_logger(poor::LogCategory::USER_ACTIVITY,"user_activity");
    create_file_logger(poor::LogCategory::NETWORK,"network");

    // 设置默认日志器
    spdlog::set_default_logger(this->loggers[poor::LogCategory::APPLICATION_ACTIVITY]);
    spdlog::set_level(spdlog::level::debug); // 设置全局日志级别
    spdlog::flush_on(spdlog::level::info); // 设置日志刷新级别
}

// 清理日志器
void LoggerManager::cleanup()
{
    this->loggers.clear();
}

// 获取日志器
std::shared_ptr<spdlog::logger> LoggerManager::getLogger(poor::LogCategory category)
{
    auto it = this->loggers.find(category);
    if(it != this->loggers.end())
    {
        return it->second;
    }
    else
    {
        // 如果日志器不存在，返回默认日志器
        return spdlog::default_logger();
    }
}

// 创建日志文件夹
void LoggerManager::Create_log_directory(rpc_server::ServerType server_type)
{
    std::string server_name = "default"; // 默认日志文件夹名

    switch(server_type) // 根据服务器类型设置日志文件夹名
    {
    case rpc_server::ServerType::CENTRAL:
    {
        server_name = "central_server";
        break;
    }
    case rpc_server::ServerType::DB:
    {
        server_name = "db_server";
        break;
    }
    case rpc_server::ServerType::GATEWAY:
    {
        server_name = "gateway_server";
        break;
    }
    case rpc_server::ServerType::LOGIN:
    {
        server_name = "login_server";
        break;
    }
    case rpc_server::ServerType::LOGIC:
    {
        server_name = "logic_server";
        break;
    }
    default:
    {
        server_name = "default";
        break;
    }
    }

    // 创建日志文件夹
    this->log_directory = "../../logs/" + server_name;
    std::filesystem::create_directories(this->log_directory);
}
