#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H

#include "common.pb.h" // 公共模块：包含公共数据类型，枚举
#include "consts/const_log.h"  // 日志分类，日志等级常量
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <ctime>

// 日志管理器类，管理所有日志
class LoggerManager
{
public:
    // 初始化日志器
    void initialize(rpc_server::ServerType server_type);
    // 清理日志器
    void cleanup();

    // 获取日志器
    std::shared_ptr<spdlog::logger> getLogger(poor::LogCategory category);

private:
    // 创建日志文件夹
    void Create_log_directory(rpc_server::ServerType server_type);

private:
    std::unordered_map<poor::LogCategory, std::shared_ptr<spdlog::logger>> loggers;   // 日志器容器
    std::string log_directory;  // 日志文件夹
};

#endif // LOGGER_MANAGER_H
