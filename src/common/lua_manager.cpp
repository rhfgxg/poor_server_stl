#include "lua_manager.h"

LuaManager::LuaManager():
    L(luaL_newstate())  // 创建lua虚拟机
{
    luaL_openlibs(L);   // 打开lua标准库
}

LuaManager::~LuaManager()
{
    lua_close(L);   // 关闭lua虚拟机
}

bool LuaManager::Run_script(const std::string& script)
{
    if(luaL_dofile(L, script.c_str()) != LUA_OK)
    {
        std::cerr << "Error running script: " << lua_tostring(L, -1) << std::endl;
        return false;
    }
    return true;
}
