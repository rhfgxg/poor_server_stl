#ifndef EVENT_H
#define EVENT_H

#include <map>
#include <functional>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>

// 定义事件枚举
enum class EventType
{
    LOGIN = 0,  // 玩家登录
    LOGOUT = 1,     // 玩家登出
};

// 事件管理器
class EventManager
{
public:
    EventManager();
    ~EventManager();

    void Register_event(EventType event, std::function<void()> callback);     // 注册无参事件回调函数
    void Register_event(EventType event, std::function<void(int)> callback);  // 注册单参事件回调函数
    void Register_event(EventType event, std::function<void(const std::string&)> callback); // 注册单参事件回调函数（字符串）
    void Register_event(EventType event, std::function<void(int, const std::string&)> callback); // 注册双参事件回调函数

    void Unregister_event(EventType event);      // 注销事件

    // 事件触发时，将回调函数作为任务加入任务队列
    void Trigger_event(EventType event);       // 触发事件，无参
    void Trigger_event(EventType event, int value);       // 触发事件，一个参数
    void Trigger_event(EventType event, const std::string& message); // 触发事件，一个字符串参数
    void Trigger_event(EventType event, int value, const std::string& message); // 触发事件，两个参数

private:
    // 事件与回调函数的映射：一个事件对应多个回调函数
    std::map<EventType, std::vector<std::function<void()>>> callbacks_no_args;
    std::map<EventType, std::vector<std::function<void(int)>>> callbacks_one_arg_int;
    std::map<EventType, std::vector<std::function<void(const std::string&)>>> callbacks_one_arg_string;
    std::map<EventType, std::vector<std::function<void(int, const std::string&)>>> callbacks_two_args;

    std::mutex mtx; // 互斥锁保护回调函数映射表
    std::condition_variable cv; // 条件变量用于线程间同步
};

#endif // !EVENT_H
