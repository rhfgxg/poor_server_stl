#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H

// lua管理器
#include <string>

class LuaManager {

public:
    LuaManager();
    ~LuaManager();
    bool Run_script(const std::string& script_name);       // 执行lua脚本

    // 注册c++函数到lua
    void registerFunction(const char* name, lua_CFunction func);
    // 注册c++变量到lua
    void registerVariable(const char* name, int value);
    // 注册c++变量到lua
    void registerVariable(const char* name, const char* value);
    // 获取lua状态
    lua_State* getState();

private:
    lua_State* L;
};


#endif // LUA_MANAGER_H
