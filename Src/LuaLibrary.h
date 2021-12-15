//
// Created by musmusliu on 2021/10/28.
//

#ifndef LUA_LIBRARY_H_
#define LUA_LIBRARY_H_

#include "lua.hpp"
#include "LuaLibraryInternal.h"

/**
 * 创建全局表
 * @param L
 * @param name
 * @param narr
 * @param nrec
 * @return
 */
int CreateGlobalTable(lua_State* L,
		const char* name,
		int narr,
		int nrec) noexcept;

/**
 * 压入Lua支持的任何值
 * @tparam T
 * @param L
 * @param any
 */
template<typename T>
void PushNative(lua_State* L,
		T any) noexcept
{
	PushNativeInternal(L, any);
}

/**
 * 压入多个值
 * @tparam Args
 * @param L
 * @param tuple
 */
template<typename... Args>
void PushNatives(lua_State* L,
		const std::tuple<Args...>& tuple) noexcept
{
	PushNativesInternal(L, tuple, std::make_index_sequence<sizeof...(Args)>{});
}

/**
 * 转成模版对应类型
 * @tparam T
 * @param L
 * @param idx
 * @return
 */
template<typename T>
T ToNative(lua_State* L, int idx)
{
	return ToNativeInternal<T>(L, idx);
}

/**
 * 依次从栈底转换这N个数
 * @tparam Args
 * @param L
 * @param tuple
 */
template<typename... Args>
void ToNatives(lua_State* L,
		std::tuple<Args...>& tuple,
		int idx = 1)
{
	ToNativesInternal(L, tuple, idx, std::make_index_sequence<sizeof...(Args)>{});
}

///**
// * 设置元方法
// * @tparam Args
// * @param L
// * @param idx
// * @param metaMethod
// * @param closure
// * @param upValues
// * @return
// */
//template<typename... Args>
//int SetMetaMethod(
//		int idx,
//		lua_State* L,
//		const char* metaMethod,
//		lua_CFunction closure,
//		Args&& ...upValues)
//{
//	int top = lua_gettop(L);
//	if (PushNative(L, metaMethod, upValues...) == 0)
//	{
//		lua_pop(L, lua_gettop(L) - top);
//		return 0;
//	}
//	lua_pushcclosure(L, closure, sizeof...(Args));
//	lua_rawset(L, idx);
//
//	return 1;
//}
//
///**
// * 设置元方法
// * @tparam Args
// * @param L
// * @param idx
// * @param metaMethod
// * @param closure
// * @param upValues
// * @return
// */
//template<typename... Args>
//int SetMetaMethod(
//		int idx,
//		lua_State* L,
//		const char* metaMethod,
//		lua_CFunction closure,
//		Args& ...upValues)
//{
//	int top = lua_gettop(L);
//	if (PushNative(L, metaMethod, upValues...) == 0)
//	{
//		lua_pop(L, lua_gettop(L) - top);
//		return 0;
//	}
//	lua_pushcclosure(L, closure, sizeof...(Args));
//	lua_rawset(L, idx);
//
//	return 1;
//}

/**
 * 创建userData
 * @tparam T
 * @tparam Args
 * @param L
 * @return
 */
template<typename T, typename... Args>
void CreateUserData(lua_State* L)
{
	if (lua_gettop(L) < sizeof...(Args))
		throw LuaException("no enough params in lua stack");

	std::tuple<Args...> tuple;
	ToNatives(L, tuple);

	T** pointer = (T**)lua_newuserdata(L, sizeof(T**));
	*pointer = CreateUserDataInternal<T>(L, tuple, std::make_index_sequence<sizeof...(Args)>{});
}

/**
 * 创建userData无参数
 * @tparam T
 * @param L
 * @return
 */
template<typename T>
void CreateUserDataNoParam(lua_State* L) noexcept
{
	T** pointer = (T**)lua_newuserdata(L, sizeof(T**));
	*pointer = new T;
}

/**
 * 调用userdata的函数，用于lua类包装方法中
 * @tparam T
 * @tparam RetType
 * @tparam ArgTypes
 * @param pointer
 * @param func
 * @param tuple
 */
template<typename T, typename RetType, typename... ArgTypes>
int HelpCallObjectFunction(lua_State* L,
		RetType(T::*func)(ArgTypes...))
{
	if (lua_gettop(L) < sizeof...(ArgTypes) + 1)
		throw LuaException("no enough params to call function");

	T** userData = ToNative<T**>(L, 1);

	std::tuple<ArgTypes...> tuple;
	ToNatives(L, tuple, 2);

	if constexpr (!std::is_void<RetType>::value)
	{
		PushNative(L, HelpCallObjectFunctionInternal(*userData, func,
				tuple, std::make_index_sequence<sizeof...(ArgTypes)>{}));
		return 1;
	}

	HelpCallObjectFunctionInternal(*userData, func,
		tuple, std::make_index_sequence<sizeof...(ArgTypes)>{});
	return 0;
}

/**
 * 调用userdata的函数，用于lua类包装方法中
 * @tparam T
 * @tparam RetType
 * @tparam ArgTypes
 * @param pointer
 * @param func
 * @param tuple
 */
template<typename T, typename RetType, typename... ArgTypes>
int HelpCallObjectFunction(lua_State* L,
		RetType(T::*func)(ArgTypes...) const)
{
	if (lua_gettop(L) < sizeof...(ArgTypes) + 1)
		throw LuaException("no enough params to call function");

	T** userData = ToNative<T**>(L, 1);

	std::tuple<ArgTypes...> tuple;
	ToNatives(L, tuple, 2);

	if constexpr (!std::is_void<RetType>::value)
	{
		PushNative(L, HelpCallObjectFunctionInternal(*userData, func,
				tuple, std::make_index_sequence<sizeof...(ArgTypes)>{}));
		return 1;
	}

	HelpCallObjectFunctionInternal(*userData, func,
		tuple, std::make_index_sequence<sizeof...(ArgTypes)>{});
	return 0;
}

/**
 * 调用userdata的函数，用于lua类包装方法中
 * @tparam T
 * @tparam RetType
 * @tparam ArgTypes
 * @param L
 * @param func
 */
template<typename RetType, typename... ArgTypes>
int HelpCallFunction(lua_State* L,
		RetType(* function)(ArgTypes...))
{
	if (lua_gettop(L) < sizeof...(ArgTypes))
		throw LuaException("no enough params to call function");

	std::tuple<ArgTypes...> tuple;
	ToNatives(L, tuple);

	if constexpr (!std::is_void<RetType>::value)
	{
		PushNative(L, HelpCallFunctionInternal(function,
				tuple, std::make_index_sequence<sizeof...(ArgTypes)>{}));
		return 1;
	}

	HelpCallFunctionInternal(function,
		tuple, std::make_index_sequence<sizeof...(ArgTypes)>{});
	return 0;
}

/**
 * 调用lua方法,有参数有返回值
 * @tparam ArgTypes
 * @tparam RetTypes
 * @param L
 * @param error
 * @param args
 * @param rets
 * @return
 */
template<typename ...ArgTypes, typename ...RetTypes>
void CallLuaFunctionParamRet(lua_State* L,
		std::tuple<RetTypes...>& rets,
		ArgTypes...args)
{
	int top = lua_gettop(L);

	std::tuple<ArgTypes...> argsTuple{ args... };
	PushNatives(L, argsTuple);
	CallLuaFunctionInternal(L, sizeof...(ArgTypes), sizeof...(RetTypes));

	ToNatives(L, rets, top);
	lua_pop(L, (int)sizeof...(RetTypes));
}

/**
 * 调用lua方法, 有参数无返回值
 * @tparam ArgTypes
 * @param L
 * @param error
 * @param args
 * @return
 */
template<typename ...ArgTypes>
void CallLuaFunctionParamNoRet(lua_State* L,
		ArgTypes...args)
{
	std::tuple<ArgTypes...> argsTuple{ args... };
	PushNatives(L, argsTuple);
	CallLuaFunctionInternal(L, sizeof...(ArgTypes), 0);
}

/**
 * 调用lua方法, 无参数有返回值
 * @tparam RetTypes
 * @param L
 * @param error
 * @param rets
 * @return
 */
template<typename ...RetTypes>
void CallLuaFunctionNoParamRet(lua_State* L,
		std::tuple<RetTypes...>& rets)
{
	int top = lua_gettop(L);

	CallLuaFunctionInternal(L, 0, sizeof...(RetTypes));

	ToNatives(L, rets, top);
	lua_pop(L, sizeof...(RetTypes));

}

/**
 * 调用lua方法, 无参数无返回值
 * @param L
 * @param error
 * @return
 */
void CallLuaFunctionNoParamNoRet(lua_State* L);

/**
 * 调用lua全局方法, 有参数有返回值
 * @tparam ArgTypes
 * @tparam RetTypes
 * @param L
 * @param functionName
 * @param error
 * @param args
 * @param rets
 * @return
 */
template<typename ...ArgTypes, typename ...RetTypes>
void CallLuaGlobalFunctionParamRet(lua_State* L,
		const char* functionName,
		std::tuple<RetTypes...>& rets,
		ArgTypes...args)
{
	lua_getglobal(L, functionName);

	CallLuaFunctionParamRet(L, rets, args...);
}

/**
 * 调用lua全局方法, 有参数无返回值
 * @tparam ArgTypes
 * @param L
 * @param functionName
 * @param error
 * @param args
 * @return
 */
template<typename ...ArgTypes>
void CallLuaGlobalFunctionParamNoRet(lua_State* L,
		const char* functionName,
		ArgTypes...args)
{
	lua_getglobal(L, functionName);
	CallLuaFunctionParamNoRet(L, args...);
}

/**
 * 调用lua全局方法, 无参数有返回值
 * @tparam RetTypes
 * @param L
 * @param functionName
 * @param error
 * @param rets
 * @return
 */
template<typename ...RetTypes>
void CallLuaGlobalFunctionNoParamRet(lua_State* L,
		const char* functionName,
		std::tuple<RetTypes...>& rets)
{
	lua_getglobal(L, functionName);
	CallLuaFunctionNoParamRet(L, rets);
}

/**
 * 调用lua全局方法, 无参数无返回值
 * @param L
 * @param functionName
 * @param error
 * @return
 */
void CallLuaGlobalFunctionNoParamNoRet(lua_State* L,
		const char* functionName);

#endif
