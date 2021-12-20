//
// Created by musmusliu on 2021/10/25.
//
#include <lua.hpp>
#include "HelloLua.h"
#include <iostream>

class Test
{
public:
	Test(int a, bool s) : _field(a)
	{
	};

	static int SField;

	int Print(const char* msg)
	{
		std::cout << "Method:" << msg << _field << std::endl;
		return 1;
	}

	int GetField() const
	{
		return _field;
	}

	void Method()
	{
		std::cout << "Methid1";
		_field = 20;
	}

	static int SPrint(const char* msg, const char* tips)
	{
		std::cout << "Static Method:" << msg << tips << std::endl;
		return 0;
	}

public:
	int _field;
	std::tuple<const char*, int(*)(lua_State*), int> t[1] = {
			{"", [](lua_State* L){
				return 1;
			}, 1}
	};

	constexpr static  int a = 1;
};

int Test::SField = 1;

void Destroy(Test* test)
{
	std::cout << "Destrpy" << std::endl;

	delete test;
}

class Test1
{
public:
	void Print1() 
	{
		std::cout << "Test1" << std::endl;
	}
};

BEGIN_DECLARE_LUA_WRAPPER(Test, 1, 1, 1, 1)
DECLARE_CONSTRUCTOR(int, bool)
DECLARE_DESTRUCTOR(Destroy)
BEGIN_DECLARE_MEMBER()
DECLARE_METHOD(GetField, int)
DECLARE_METHOD(Method, void)
DECLARE_STATIC_METHOD(SPrint, int, const char*, const char*)
DECLARE_STATIC_FIELD(SField)
DECLARE_METHOD(Print, int, const char*)
DECLARE_FIELD(_field)
END_DECLARE_MEMBER()
END_DECLARE_LUA_WRAPPER()

BEGIN_DECLARE_LUA_WRAPPER(Test1, 0, 0, 1, 0)
DECLARE_EMPTY_CONSTRUCTOR()
DECLARE_EMPTY_DESTRUCTOR()
BEGIN_DECLARE_MEMBER()
DECLARE_METHOD(Print1, void)
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

int Index (lua_State* L)
{
	auto method = [](lua_State* L){
		Test** test = (Test**)lua_touserdata(L, 1);
		return (*test)->_field;
	};

	return method(L);
}

int main(int argc, char** argv)
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	REGISTER_CLASS(Test, L);
	REGISTER_CLASS(Test1, L);

	int ret = luaL_dofile(L, "Lua/Main.lua");
	if (ret != 0)
	{
		std::cout << lua_tostring(L, -1);
	}

	lua_pushstring(L, "ss");

	Test* test = new Test(10, false);
	Test1* test1 = new Test1;

	try 
	{
		CallLuaGlobalFunctionParamNoRet(L, "Main", &test, &test1);
	}
	catch (LuaException& e)
	{
		std::cout << e.what() << std::endl;
	}

	delete test1;
	delete test;
	lua_close(L);

	return 0;
}