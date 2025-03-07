#ifndef CONST_LOG_H
#define CONST_LOG_H

// 日志相关枚举

namespace rpc_server
{

// 日志分级
enum LogLevel
{
    DEBUG = 0,  // debug
    INFO = 1,   // 正常信息
    WARN = 2,   // 警告
    ERROR_LEVEL = 3,    // 异常，ERROR 是宏定义，所以使用 ERROR_LEVEL代替
    CRITICAL = 4,  // 致命错误
    TRACE = 5   // 跟踪
};

// 日志分类
enum LogCategory
{
    STARTUP_SHUTDOWN = 0,       // 启动和关闭
    APPLICATION_ACTIVITY = 1,   // 应用程序活动（请求、响应、调试）
    CONNECTION_POOL = 2,        // 连接池
    SYSTEM_MONITORING = 3,      // 系统监控（性能监控、定时任务）
    HEARTBEAT = 4,              // 心跳
    SECURITY = 5,               // 安全
    CONFIGURATION_CHANGES = 6,  // 配置变更
    DATABASE_OPERATIONS = 7,    // 数据库操作
    USER_ACTIVITY = 8,          // 用户活动
    NETWORK = 9                // 网络
};

}



#endif // CONST_LOG_H


