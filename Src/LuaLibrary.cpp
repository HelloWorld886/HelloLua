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
	long any) noexcept
{
	lua_pushinteger(L, (lua_Integer)any);
}

template<>
void PushNativeInternal(lua_State* L,
		std::string any) noexcept
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
		int idx) noexcept
{
	return lua_toboolean(L, idx) != 0;
}

template<>
int ToNativeInternal(lua_State* L,
		int idx) noexcept
{
	return (int)lua_tointeger(L, idx);
}

template<>
long ToNativeInternal(lua_State* L,
	int idx) noexcept
{
	return (long)lua_tointeger(L, idx);
}

template<>
std::string ToNativeInternal(lua_State* L,
		int idx) noexcept
{
	const char* str = lua_tostring(L, idx);
	if (str == nullptr)
		return "";

	return str;
}

template<>
double ToNativeInternal(lua_State* L,
		int idx) noexcept
{
	return lua_tonumber(L, idx);
}

template<>
float ToNativeInternal(lua_State* L,
		int idx) noexcept
{
	return (float)lua_tonumber(L, idx);
}

template<>
const char* ToNativeInternal(lua_State* L,
		int idx) noexcept
{
	return lua_tostring(L, idx);
}

template<>
lua_CFunction ToNativeInternal(lua_State* L,
		int idx) noexcept
{
	return lua_tocfunction(L, idx);
}

void CallLuaFunctionInternal(lua_State* L,
		int argCount,
		int retCount)
{
	if(lua_pcall(L, argCount, retCount, 0) != 0)
	{
		const char* error = lua_tostring(L, -1);
		lua_pop(L, 1);
		throw LuaException(error);
	}
}

void CallLuaFunctionNoParamNoRet(lua_State* L)
{
	CallLuaFunctionInternal(L, 0, 0);
}

void CallLuaGlobalFunctionNoParamNoRet(lua_State* L,
		const char* functionName)
{
	lua_getglobal(L, functionName);
	CallLuaFunctionNoParamNoRet(L);
}

void PrintStackDump(lua_State* L)
{
	int i;
	int top = lua_gettop(L);
	//从栈底到栈顶遍历了整个堆栈，依照每个元素自己的类型打印出其值。 
	for (i = 1; i <= top; i++) { /* repeat for each level */
		int t = lua_type(L, i);
		switch (t)
		{
		case LUA_TSTRING: /* strings */
			printf("`%s'", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN: /* booleans */
			printf(lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TNUMBER: /* numbers */
			printf("%g", lua_tonumber(L, i));
			break;
		default: /* other values */
			printf("%s", lua_typename(L, t));
			break;
		}
		printf(" "); /* put a separator */
	}
	printf("\n"); /* end the listing */
}
