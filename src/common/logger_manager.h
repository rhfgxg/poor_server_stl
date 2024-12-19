#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <filesystem>

// 日志分级
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
	ERROR_LEVEL,    // ERROR 是宏定义，所以使用 ERROR_LEVEL
    FATAL,
    TRACE
};

// 日志分类
enum class LogCategory {
    STARTUP_SHUTDOWN,
    REQUESTS_RESPONSES,
    ERRORS_EXCEPTIONS,
    CONNECTION_POOL,
    PERFORMANCE_MONITORING,
    SECURITY,
    DEBUGGING,
    CONFIGURATION_CHANGES,
    SCHEDULED_TASKS
};

class LoggerManager
{
public:
    // 初始化日志器
    void initialize(const std::string& server_name);

    // 获取日志器
    std::shared_ptr<spdlog::logger> getLogger(LogCategory category);

private:
    // 创建日志文件夹
    void createLogDirectory(const std::string& server_name);

    std::unordered_map<LogCategory, std::shared_ptr<spdlog::logger>> loggers;   // 日志器容器
    std::string log_directory;  // 日志文件夹
};

#endif // LOGGER_MANAGER_H

