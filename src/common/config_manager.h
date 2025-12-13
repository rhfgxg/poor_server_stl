#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <optional>
#include <vector>
#include <lua.hpp>

// 简易配置管理器：从多个可能路径加载 lua 表，并提供查询方法
class ConfigManager
{
public:
    ConfigManager() = default;
    ~ConfigManager();

    // 尝试从多个可能路径加载配置文件，返回加载的路径或空
    std::optional<std::string> load_from_paths(const std::vector<std::string>& paths);

    // 从已加载的配置中获取字符串或整数
    std::optional<std::string> get_string(const std::string& key) const;
    std::optional<int> get_int(const std::string& key) const;

private:
    void reset();
    bool push_value_by_path(const std::string& key) const;

private:
    lua_State* L_ = nullptr;
    int table_ref_ = LUA_NOREF;
    std::optional<std::string> table_path_; // 记录加载的文件路径
};

#endif // CONFIG_MANAGER_H
