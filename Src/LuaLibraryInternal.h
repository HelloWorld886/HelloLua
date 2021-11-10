//
// Created by musmusliu on 2021/11/10.
//

#pragma once

#include <tuple>
#include <functional>
#include <type_traits>
#include <string>
#include <stdexcept>

class LuaException : public std::runtime_error
{
public:
	explicit LuaException(const std::string& msg) :
			std::runtime_error(msg){}
};

template<typename... Args, size_t... Integers>
void PushNativesInternal(lua_State* L,
		std::tuple<Args...>& tuple,
		std::index_sequence<Integers...>&&) noexcept
{
	std::initializer_list<int>{ (PushNativeInternal<Args>(L, std::get<Integers>(tuple)), 0)... };
}

template<typename T>
void PushNativeInternal(lua_State* L,
		const T& any) noexcept
{
	if constexpr (std::is_same<T, bool>::value)
		lua_pushboolean(L, (bool)any);
	else if constexpr (std::is_integral<T>::value)
		lua_pushinteger(L, (lua_Integer)any);
	else if constexpr (std::is_same<T, std::string>::value)
		lua_pushstring(L, any.c_str());
	else if constexpr (std::is_floating_point<T>::value)
		lua_pushnumber(L, (lua_Number)any);
	else if constexpr (std::is_pointer<T>::value)
	{
		using Type = std::remove_cv<std::remove_pointer<T>::type>::type;
		if (std::is_same<Type, char>::value)
		{
			if (any != nullptr)
				lua_pushstring(L, (const char*)any);
			else
				lua_pushnil(L);
		}
	}
	else if constexpr (std::is_function<T>::value)
		lua_pushcfunction(L, any);
	else
	{
		lua_pushnil(L);
	}
}

template<typename T>
T ToNativeInternal(lua_State* L,
		int idx)
{
	if constexpr (std::is_same<T, bool>::value)
	{
		if(lua_type(L, idx) != LUA_TBOOLEAN)
			throw LuaException("can't cast to boolean");
		return lua_toboolean(L, idx);
	}
	else if constexpr (std::is_integral<T>::value)
	{
		if(lua_type(L, idx) != LUA_TNUMBER)
			throw LuaException("can't cast to integer");
		return lua_tointeger(L, idx);
	}
	else if constexpr (std::is_same<T, std::string>::value)
	{
		if(lua_type(L, idx) != LUA_TSTRING)
			throw LuaException("can't cast to std::string");
		return std::string(lua_tostring(L, idx));
	}
	else if constexpr (std::is_floating_point<T>::value)
	{
		if(lua_type(L, idx) != LUA_TNUMBER)
			throw LuaException("can't cast to float point");
		return lua_tonumber(L, idx);
	}
	else if constexpr (std::is_pointer<T>::value)
	{
		using Type = std::remove_volatile<std::remove_pointer<T>::type>::type;
		if constexpr (std::is_same<Type, const char>::value)
		{
			if(lua_type(L, idx) != LUA_TSTRING)
				throw LuaException("can't cast to const char*");
			return lua_tostring(L, idx);
		}
		else if constexpr (std::is_same<T, lua_CFunction>::value)
		{
			if(lua_type(L, idx) != LUA_TFUNCTION)
				throw LuaException("can't cast to lua_CFunction");
			return lua_tocfunction(L, idx);
		}
		else
		{
			if(lua_type(L, idx) != LUA_TUSERDATA)
				throw LuaException("can't cast to userdata");
			return (T)lua_touserdata(L, idx);
		}
	}
}

template<typename... Args, size_t... Integers>
void ToNativesInternal(lua_State* L,
		std::tuple<Args...>& tuple,
		int idx,
		std::index_sequence<Integers...>&&)
{
	idx = lua_absindex(L, idx);
	int _[] = { 0, (std::get<Integers>(tuple) = ToNativeInternal<Args>(L, (int)Integers + idx), 0)... };
}

template<typename T, typename... Args, size_t... Integers>
T* CreateUserDataInternal(lua_State* L,
		std::tuple<Args...>& tuple,
		std::index_sequence<Integers...>&&)
{
	T* instance = new T(std::get<Integers>(tuple)...);
	return instance;
}

template<typename T, typename RetType, typename... ArgTypes, size_t... Integers>
RetType HelpCallObjectFunctionInternal(T* pointer,
		RetType(T::*func)(ArgTypes...),
		std::tuple<ArgTypes...>& tuple,
		std::index_sequence<Integers...>&&)
{
	return (pointer->*func)(std::get<Integers>(tuple)...);
}

template<typename RetType, typename... ArgTypes, size_t... Integers>
RetType HelpCallFunctionInternal(
		std::function<RetType(ArgTypes...)> function,
		std::tuple<ArgTypes...>& tuple,
		std::index_sequence<Integers...>&&)
{
	return function(std::get<Integers>(tuple)...);
}
