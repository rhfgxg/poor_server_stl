#include "logger_manager.h"
#include <array>

namespace
{
struct CategoryConfig
{
    poor::LogCategory category;
    const char* folder;
};

constexpr std::array<CategoryConfig, 10> kCategoryConfigs = {{
    {poor::LogCategory::STARTUP_SHUTDOWN, "startup_shutdown"},
    {poor::LogCategory::APPLICATION_ACTIVITY, "application_activity"},
    {poor::LogCategory::CONNECTION_POOL, "connection_pool"},
    {poor::LogCategory::SYSTEM_MONITORING, "system_monitoring"},
    {poor::LogCategory::HEARTBEAT, "heartbeat"},
    {poor::LogCategory::SECURITY, "security"},
    {poor::LogCategory::CONFIGURATION_CHANGES, "configuration_changes"},
    {poor::LogCategory::DATABASE_OPERATIONS, "database_operations"},
    {poor::LogCategory::USER_ACTIVITY, "user_activity"},
    {poor::LogCategory::NETWORK, "network"}
}};
}

// 初始化日志器
void LoggerManager::initialize(rpc_server::ServerType server_type)
{
    if (!this->loggers.empty())
    {
        this->cleanup();
    }

    // 创建日志文件夹
    this->Create_log_directory(server_type);

    // 创建控制台日志器
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");

    // 创建不同分类的文件日志器
    auto create_file_logger = [&](poor::LogCategory category, const std::string& log_type) {
        const auto category_dir = std::filesystem::path(this->log_directory) / log_type;
        std::filesystem::create_directories(category_dir);

        auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>((category_dir / "log.log").string(), 0, 0); // 按日期滚动
        file_sink->set_level(spdlog::level::debug);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");

        spdlog::sinks_init_list sink_list = {console_sink,file_sink};
        auto logger = std::make_shared<spdlog::logger>(std::to_string(static_cast<int>(category)), sink_list.begin(), sink_list.end());
        logger->set_level(spdlog::level::debug);
        spdlog::register_logger(logger);
        this->loggers[category] = std::move(logger);
    };

    // 创建所有分类的日志器
    for (const auto& config : kCategoryConfigs)
    {
        create_file_logger(config.category, config.folder);
    }

    // 设置默认日志器
    spdlog::set_default_logger(this->loggers[poor::LogCategory::APPLICATION_ACTIVITY]);
    spdlog::set_level(spdlog::level::debug); // 设置全局日志级别
    spdlog::flush_on(spdlog::level::info); // 设置日志刷新级别
}

// 清理日志器
void LoggerManager::cleanup()
{
    for (auto& entry : this->loggers)
    {
        auto& logger = entry.second;
        if (logger)
        {
            logger->flush();
            spdlog::drop(logger->name());
        }
    }

    this->loggers.clear();
    spdlog::shutdown();
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
    const auto directory_path = std::filesystem::path("../../logs") / server_name;
    std::filesystem::create_directories(directory_path);
    this->log_directory = directory_path.string();
}
