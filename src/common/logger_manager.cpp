#include "logger_manager.h"
#include <ctime>

void LoggerManager::initialize(const std::string& server_name)
{
    // 创建日志文件夹
    createLogDirectory(server_name);

    // 创建控制台日志器
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");

    // 创建不同分类的文件日志器
    auto create_file_logger = [&](LogCategory category, const std::string& filename) {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_directory + "/" + filename, true);
        file_sink->set_level(spdlog::level::debug);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");

        spdlog::sinks_init_list sink_list = { console_sink, file_sink };
        auto logger = std::make_shared<spdlog::logger>(std::to_string(static_cast<int>(category)), sink_list.begin(), sink_list.end());
        logger->set_level(spdlog::level::debug);
        loggers[category] = logger;
        };

    create_file_logger(LogCategory::STARTUP_SHUTDOWN, "startup_shutdown.log");
    create_file_logger(LogCategory::REQUESTS_RESPONSES, "requests_responses.log");
    create_file_logger(LogCategory::ERRORS_EXCEPTIONS, "errors_exceptions.log");
    create_file_logger(LogCategory::CONNECTION_POOL, "connection_pool.log");
    create_file_logger(LogCategory::PERFORMANCE_MONITORING, "performance_monitoring.log");
    create_file_logger(LogCategory::SECURITY, "security.log");
    create_file_logger(LogCategory::DEBUGGING, "debugging.log");
    create_file_logger(LogCategory::CONFIGURATION_CHANGES, "configuration_changes.log");
    create_file_logger(LogCategory::SCHEDULED_TASKS, "scheduled_tasks.log");

    // 设置默认日志器
    spdlog::set_default_logger(loggers[LogCategory::DEBUGGING]);
    spdlog::set_level(spdlog::level::debug); // 设置全局日志级别
    spdlog::flush_on(spdlog::level::info); // 设置日志刷新级别
}

std::shared_ptr<spdlog::logger> LoggerManager::getLogger(LogCategory category)
{
    auto it = loggers.find(category);
    if (it != loggers.end())
    {
        return it->second;
    }
    else
    {
        // 如果日志器不存在，返回默认日志器
        return spdlog::default_logger();
    }
}

void LoggerManager::createLogDirectory(const std::string& server_name)
{
    // 获取当前日期
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    char date[11];
    std::strftime(date, sizeof(date), "%Y/%m/%d", &tm);

    // 创建日志文件夹
    log_directory = "logs/" + server_name + "/" + date;
    std::filesystem::create_directories(log_directory);
}