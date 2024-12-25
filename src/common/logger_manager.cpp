#include "logger_manager.h"
#include <ctime>
#include <spdlog/sinks/daily_file_sink.h>

void LoggerManager::initialize(myproject::ServerType server_type)
{
    // 创建日志文件夹
    createLogDirectory(server_type);

    // 创建控制台日志器
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");

    // 创建不同分类的文件日志器
    auto create_file_logger = [&](LogCategory category,const std::string& log_type) {
        auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_directory + "/" + log_type + "/log.log",0,0); // 按日期滚动
        file_sink->set_level(spdlog::level::debug);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");

        spdlog::sinks_init_list sink_list = {console_sink,file_sink};
        auto logger = std::make_shared<spdlog::logger>(std::to_string(static_cast<int>(category)),sink_list.begin(),sink_list.end());
        logger->set_level(spdlog::level::debug);
        loggers[category] = logger;
    };

    create_file_logger(LogCategory::STARTUP_SHUTDOWN,"startup_shutdown");
    create_file_logger(LogCategory::REQUESTS_RESPONSES,"requests_responses");
    create_file_logger(LogCategory::ERRORS_EXCEPTIONS,"errors_exceptions");
    create_file_logger(LogCategory::CONNECTION_POOL,"connection_pool");
    create_file_logger(LogCategory::PERFORMANCE_MONITORING,"performance_monitoring");
    create_file_logger(LogCategory::SECURITY,"security");
    create_file_logger(LogCategory::DEBUGGING,"debugging");
    create_file_logger(LogCategory::CONFIGURATION_CHANGES,"configuration_changes");
    create_file_logger(LogCategory::SCHEDULED_TASKS,"scheduled_tasks");

    // 设置默认日志器
    spdlog::set_default_logger(loggers[LogCategory::DEBUGGING]);
    spdlog::set_level(spdlog::level::debug); // 设置全局日志级别
    spdlog::flush_on(spdlog::level::info); // 设置日志刷新级别
}

// 清理日志器
void LoggerManager::cleanup()
{
    loggers.clear();
}

// 获取日志器
std::shared_ptr<spdlog::logger> LoggerManager::getLogger(LogCategory category)
{
    auto it = loggers.find(category);
    if(it != loggers.end())
    {
        return it->second;
    } else
    {
        // 如果日志器不存在，返回默认日志器
        return spdlog::default_logger();
    }
}

// 创建日志文件夹
void LoggerManager::createLogDirectory(myproject::ServerType server_type)
{
    std::string server_name = "default"; // 默认日志文件夹名

    switch(server_type)
    {
    case myproject::CENTRAL:
    server_name = "central_server";
    break;
    case myproject::DATA:
    server_name = "data_server";
    break;
    case myproject::GATEWAY:
    server_name = "gateway_server";
    break;
    case myproject::LOGIN:
    server_name = "login_server";
    break;
    case myproject::LOGIC:
    server_name = "logic_server";
    break;
    default:
    server_name = "default";
    break;
    }

    // 创建日志文件夹
    log_directory = "../../logs/" + server_name;
    std::filesystem::create_directories(log_directory);
}
