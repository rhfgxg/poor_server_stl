#include "config_manager.h"
#include <vector>
#include <iostream>

ConfigManager::~ConfigManager()
{
    if (L_)
    {
        lua_close(L_);
        L_ = nullptr;
    }
}

std::optional<std::string> ConfigManager::load_from_paths(const std::vector<std::string>& paths)
{
    if (L_)
    {
        lua_close(L_);
        L_ = nullptr;
    }

    L_ = luaL_newstate();
    if (!L_) return std::nullopt;
    luaL_openlibs(L_);

    for (const auto& p : paths)
    {
        if (luaL_dofile(L_, p.c_str()) == LUA_OK)
        {
            table_path_ = p;
            return table_path_;
        }
        lua_pop(L_, 1); // 清理错误信息
    }

    // 未找到任何配置文件
    lua_close(L_);
    L_ = nullptr;
    return std::nullopt;
}

std::optional<std::string> ConfigManager::get_string(const std::string& key) const
{
    if (!L_ || !lua_istable(L_, -1)) return std::nullopt;
    lua_getfield(L_, -1, key.c_str());
    if (lua_isstring(L_, -1))
    {
        std::string v = lua_tostring(L_, -1);
        lua_pop(L_, 1);
        return v;
    }
    lua_pop(L_, 1);
    return std::nullopt;
}

std::optional<int> ConfigManager::get_int(const std::string& key) const
{
    if (!L_ || !lua_istable(L_, -1)) return std::nullopt;
    lua_getfield(L_, -1, key.c_str());
    if (lua_isinteger(L_, -1))
    {
        int v = static_cast<int>(lua_tointeger(L_, -1));
        lua_pop(L_, 1);
        return v;
    }
    else if (lua_isnumber(L_, -1))
    {
        int v = static_cast<int>(lua_tonumber(L_, -1));
        lua_pop(L_, 1);
        return v;
    }
    lua_pop(L_, 1);
    return std::nullopt;
}
