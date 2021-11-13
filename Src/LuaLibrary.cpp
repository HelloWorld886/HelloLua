//
// Created by musmusliu on 2021/11/10.
//

#include "LuaLibrary.h"

int CreateGlobalTable(lua_State* L, const char* name, int narr, int nrec) noexcept
{
	if (lua_getfield(L, LUA_REGISTRYINDEX, name) != LUA_TNIL)
		return 0;

	lua_pop(L, 1);
	lua_createtable(L, narr, nrec);
	lua_pushvalue(L, -1);
	lua_setfield(L, LUA_REGISTRYINDEX, name);
	lua_pushstring(L, name);
	lua_setfield(L, -2, "__name");
	return 0;
}

template<>
void PushNativeInternal(lua_State* L,
		bool any) noexcept
{
	lua_pushboolean(L, any);
}

template<>
void PushNativeInternal(lua_State* L,
		int any) noexcept
{
	lua_pushinteger(L, (lua_Integer)any);
}

template<>
void PushNativeInternal(lua_State* L,
		std::string& any) noexcept
{
	lua_pushstring(L, any.c_str());
}

template<>
void PushNativeInternal(lua_State* L,
		double any) noexcept
{
	lua_pushnumber(L, (lua_Number)any);
}

template<>
void PushNativeInternal(lua_State* L,
		float any) noexcept
{
	lua_pushnumber(L, (lua_Number)any);
}

template<>
void PushNativeInternal(lua_State* L,
		const char* any) noexcept
{
	if (any != nullptr)
		lua_pushstring(L, (const char*)any);
	else
		lua_pushnil(L);
}

template<>
void PushNativeInternal(lua_State* L,
		lua_CFunction any) noexcept
{
	lua_pushcfunction(L, any);
}

template<>
bool ToNativeInternal(lua_State* L,
		int idx)
{
	if(lua_type(L, idx) != LUA_TBOOLEAN)
		throw LuaException("can't cast to boolean");
	return lua_toboolean(L, idx);
}

template<>
int ToNativeInternal(lua_State* L,
		int idx)
{
	if(lua_type(L, idx) != LUA_TNUMBER)
		throw LuaException("can't cast to integer");
	return (int)lua_tointeger(L, idx);
}

template<>
std::string ToNativeInternal(lua_State* L,
		int idx)
{
	if(lua_type(L, idx) != LUA_TSTRING)
		throw LuaException("can't cast to std::string");
	return std::string(lua_tostring(L, idx));
}

template<>
double ToNativeInternal(lua_State* L,
		int idx)
{
	if(lua_type(L, idx) != LUA_TNUMBER)
		throw LuaException("can't cast to double");
	return lua_tonumber(L, idx);
}

template<>
float ToNativeInternal(lua_State* L,
		int idx)
{
	if(lua_type(L, idx) != LUA_TNUMBER)
		throw LuaException("can't cast to float");
	return (float)lua_tonumber(L, idx);
}

template<>
const char* ToNativeInternal(lua_State* L,
		int idx)
{
	if(lua_type(L, idx) != LUA_TSTRING)
		throw LuaException("can't cast to const char*");
	return lua_tostring(L, idx);
}

template<>
lua_CFunction ToNativeInternal(lua_State* L,
		int idx)
{
	if(lua_type(L, idx) != LUA_TFUNCTION)
		throw LuaException("can't cast to lua_CFunction");
	return lua_tocfunction(L, idx);
}

bool CallLuaFunctionInternal(lua_State* L,
		int argCount,
		int retCount,
		std::string& error) noexcept
{
	int funcIdx = lua_gettop(L) - argCount;
	if (funcIdx < 1 || !lua_isfunction(L, funcIdx))
	{
		error = "can't find a function";
		return false;
	}

	if(lua_pcall(L, argCount, retCount, 0) != 0)
	{
		error = lua_tostring(L, -1);
		lua_pop(L, 1);
		return false;
	}

	return true;
}

bool CallLuaFunctionNoParamNoRet(lua_State* L,
		std::string& error) noexcept
{
	return CallLuaFunctionInternal(L, 0, 0, error);
}

bool CallLuaGlobalFunctionNoParamNoRet(lua_State* L,
		const char* functionName,
		std::string& error) noexcept
{
	lua_getglobal(L, functionName);
	return CallLuaFunctionNoParamNoRet(L, error);
}