#include "config_manager.h"

ConfigManager::~ConfigManager()
{
    reset();
}

void ConfigManager::reset()
{
    if (L_)
    {
        if (table_ref_ != LUA_NOREF)
        {
            luaL_unref(L_, LUA_REGISTRYINDEX, table_ref_);
            table_ref_ = LUA_NOREF;
        }
        lua_close(L_);
        L_ = nullptr;
    }

    table_path_.reset();
}

std::optional<std::string> ConfigManager::load_from_paths(const std::vector<std::string>& paths)
{
    reset();

    L_ = luaL_newstate();
    if (!L_)
    {
        return std::nullopt;
    }
    luaL_openlibs(L_);

    for (const auto& path : paths)
    {
        if (luaL_dofile(L_, path.c_str()) != LUA_OK)
        {
            lua_pop(L_, 1); // 移除错误信息
            continue;
        }

        if (!lua_istable(L_, -1))
        {
            lua_settop(L_, 0);
            continue;
        }

        table_ref_ = luaL_ref(L_, LUA_REGISTRYINDEX);
        lua_settop(L_, 0);
        table_path_ = path;
        return table_path_;
    }

    reset();
    return std::nullopt;
}

std::optional<std::string> ConfigManager::get_string(const std::string& key) const
{
    if (!push_value_by_path(key))
    {
        return std::nullopt;
    }

    std::optional<std::string> value;
    if (lua_isstring(L_, -1))
    {
        value = lua_tostring(L_, -1);
    }
    lua_pop(L_, 1);
    return value;
}

std::optional<int> ConfigManager::get_int(const std::string& key) const
{
    if (!push_value_by_path(key))
    {
        return std::nullopt;
    }

    std::optional<int> value;
    if (lua_isinteger(L_, -1))
    {
        value = static_cast<int>(lua_tointeger(L_, -1));
    }
    else if (lua_isnumber(L_, -1))
    {
        value = static_cast<int>(lua_tonumber(L_, -1));
    }

    lua_pop(L_, 1);
    return value;
}

bool ConfigManager::push_value_by_path(const std::string& key) const
{
    if (!L_ || table_ref_ == LUA_NOREF || key.empty())
    {
        return false;
    }

    lua_rawgeti(L_, LUA_REGISTRYINDEX, table_ref_);

    size_t start = 0;
    while (true)
    {
        size_t dot = key.find('.', start);
        std::string token = (dot == std::string::npos) ? key.substr(start) : key.substr(start, dot - start);
        if (token.empty())
        {
            lua_pop(L_, 1);
            return false;
        }

        lua_getfield(L_, -1, token.c_str());
        lua_remove(L_, -2); // 移除上一层表，仅保留当前值

        if (dot == std::string::npos)
        {
            return true;
        }

        if (!lua_istable(L_, -1))
        {
            lua_pop(L_, 1);
            return false;
        }

        start = dot + 1;
    }
}
