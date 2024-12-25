#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H

// 日志管理器类，管理所有日志

#include "server_central.grpc.pb.h" // 使用服务器类型枚举

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <filesystem>

// 日志分级
enum class LogLevel {
    DEBUG,  // debug
    INFO,   // 正常信息
    WARN,   // 警告
    ERROR_LEVEL,    // 异常，ERROR 是宏定义，所以使用 ERROR_LEVEL
    FATAL,  // 致命错误
    TRACE   // 跟踪
};

// 日志分类
enum class LogCategory {
    STARTUP_SHUTDOWN,   // 启动和关闭
    REQUESTS_RESPONSES, // 请求和响应
    ERRORS_EXCEPTIONS,  // 错误和异常
    CONNECTION_POOL,    // 连接池
    PERFORMANCE_MONITORING, // 性能监控
    SECURITY,   // 安全
    DEBUGGING,  // 调试
    CONFIGURATION_CHANGES,  // 配置变更
    SCHEDULED_TASKS // 定时任务
};

class LoggerManager
{
public:
    // 初始化日志器
    void initialize(myproject::ServerType server_type);
    // 清理日志器
    void cleanup();

    // 获取日志器
    std::shared_ptr<spdlog::logger> getLogger(LogCategory category);

private:
    // 创建日志文件夹
    void createLogDirectory(myproject::ServerType server_type);

    std::unordered_map<LogCategory, std::shared_ptr<spdlog::logger>> loggers;   // 日志器容器
    std::string log_directory;  // 日志文件夹
};

#endif // LOGGER_MANAGER_H

