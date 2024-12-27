#include <iostream>
#include <lua.hpp>

int main()
{
	std::cout << "main module" << std::endl;

    // 创建一个新的 Lua 状态
    lua_State* L = luaL_newstate();

    // 打开 Lua 标准库
    luaL_openlibs(L);

    // 执行 Lua 脚本
    if(luaL_dofile(L,"D:/Project/cpp/APP/poor/poor_server_stl/tools/tast.lua") != LUA_OK)
    {
        std::cerr << "Error: " << lua_tostring(L,-1) << std::endl;
        lua_pop(L,1);  // 弹出错误消息
    }

    // 关闭 Lua 状态
    lua_close(L);

	return 0;
}
