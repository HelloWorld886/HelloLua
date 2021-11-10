//
// Created by musmusliu on 2021/10/25.
//
#include <lua.hpp>
#include "HelloLua.h"
#include <iostream>

class Test
{
public:
	Test(int a,bool s) : _field(a){};

	static int SField;

	int Print(const char* msg)
	{
		std::cout << "Method:" << msg << _field << std::endl;
		return 1;
	}

	static int SPrint(const char* msg, const char* tips)
	{
		std::cout << "Static Method:" << msg << tips << std::endl;
		return 0;
	}
private:
	int _field;
};

int Test::SField = 1;

BEGIN_DECLARE_LUA_WRAPPER(Test, 1, 1, 1, 1)
DECLARE_CONSTRUCTOR(int, bool)
DECLARE_DEFAULT_DESTRUCTOR()
BEGIN_DECLARE_MEMBER()
DECLARE_STATIC_METHOD(SPrint, int, const char*, const char*)
DECLARE_STATIC_FIELD(SField)
DECLARE_METHOD(Print, int, const char*)
END_DECLARE_MEMBER()
END_DECLARE_LUA_WRAPPER()

static int index = 0;
int RealPair(lua_State* L)
{
	int parameterCount = lua_gettop(L);

	if (index < 3)
	{
		lua_pushinteger(L, index++);
		lua_pushinteger(L, 1);
	}
	else
	{
		index = 0;
		lua_pushnil(L);
	}

	return 2;
}

int Pair(lua_State* L)
{
	lua_pushcfunction(L, RealPair);
	lua_pushvalue(L, 1);
	lua_pushnil(L);

	return 3;
}

int main(int argc, char** argv)
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	REGISTER_CLASS(Test,L);

//	lua_createtable(L,0 ,0);
//	lua_createtable(L, 0,0);
//	lua_pushstring(L, "__pairs");
//	lua_pushcfunction(L, Pair);
//	lua_rawset(L, -3);
//	lua_setmetatable(L, -2);
//	lua_setglobal(L, "T");

	int ret = luaL_dofile(L, "Lua/Main.lua");
	if (ret != 0)
	{
		std::cout << lua_tostring(L, -1);
	}

	lua_close(L);

	return 0;
}