//
// Created by musmusliu on 2021/11/10.
//

#ifndef LUA_LIBRARY_INTERNAL_H_
#define LUA_LIBRARY_INTERNAL_H_

#include <tuple>
#include <functional>
#include <type_traits>
#include <string>
#include <stdexcept>

#define GET_NAME(t) #t

#define MAX_OBJECT_META_NAME_LEN 125

class LuaException : public std::runtime_error
{
public:
	explicit LuaException(const std::string& msg) :
			std::runtime_error(msg){}
};

template<typename... Args, size_t... Integers>
void PushNativesInternal(lua_State* L,
		const std::tuple<Args...>& tuple,
		std::index_sequence<Integers...>&&) noexcept
{
	std::initializer_list<int>{ (PushNativeInternal<Args>(L, std::get<Integers>(tuple)), 0)... };
}

template<typename T>
struct RemoveAllPointer
{
		using type = T;
};

template<typename T>
struct RemoveAllPointer<T*>
{
	using type = T;
};

template<typename T>
struct RemoveAllPointer<T**>
{
	using type = T;
};

template<typename T>
struct RemoveAllPointer<T***>
{
	using type = T;
};

template<typename T>
void PushObject(lua_State* L,
		T obj) noexcept
{
	static_assert(std::is_pointer<T>::value, "T is not a pointer type");
//	static_assert(std::is_pointer<std::remove_pointer<T>::type>::value, "T is not a pointer of pointer type");

	if(obj == nullptr)
	{
		lua_pushnil(L);
		return;
	}

	using Type = std::remove_cv<RemoveAllPointer<T>::type>::type;
	char metaName[MAX_OBJECT_META_NAME_LEN];
	int len = snprintf(metaName, MAX_OBJECT_META_NAME_LEN, "%s_Meta", typeid(Type).name());
	if ( len < 0 || len > MAX_OBJECT_META_NAME_LEN )
	{
		lua_pushnil(L);
		return;
	}

	lua_newtable(L);
	lua_pushstring(L, "__pointer");
	lua_pushlightuserdata(L, obj);
	lua_rawset(L, -3);
	if(lua_getfield(L, LUA_REGISTRYINDEX, metaName) == LUA_TNIL)
	{
		lua_pop(L, 1);
		return;
	}

	lua_setmetatable(L, -2);
}

//template<typename T>
//void PushNativeInternal(lua_State* L,
//		T any) noexcept
//{
//	if constexpr (std::is_same<T, bool>::value)
//		lua_pushboolean(L, (bool)any);
//	else if constexpr (std::is_integral<T>::value)
//		lua_pushinteger(L, (lua_Integer)any);
//	else if constexpr (std::is_same<T, std::string>::value)
//		lua_pushstring(L, any.c_str());
//	else if constexpr (std::is_floating_point<T>::value)
//		lua_pushnumber(L, (lua_Number)any);
//	else if constexpr (std::is_pointer<T>::value)
//	{
//		using Type = std::remove_cv<std::remove_pointer<T>::type>::type;
//		if (std::is_same<Type, char>::value)
//		{
//			if (any != nullptr)
//				lua_pushstring(L, (const char*)any);
//			else
//				lua_pushnil(L);
//		}
//		else
//			PushObject(L, any);
//	}
//	else if constexpr (std::is_function<T>::value)
//		lua_pushcfunction(L, any);
//	else
//		throw LuaException(std::string("unsupported type ").append(typeid(any).name()));
//}

template<typename T>
void PushNativeInternal(lua_State* L,
		T any) noexcept
{
	PushObject<T>(L, any);
}

template<>
void PushNativeInternal(lua_State* L,
		bool any) noexcept;

template<>
void PushNativeInternal(lua_State* L,
		int any) noexcept;

template<>
void PushNativeInternal(lua_State* L,
	long any) noexcept;

template<>
void PushNativeInternal(lua_State* L,
		std::string any) noexcept;

template<>
void PushNativeInternal(lua_State* L,
		double any) noexcept;

template<>
void PushNativeInternal(lua_State* L,
		float any) noexcept;

template<>
void PushNativeInternal(lua_State* L,
		const char* any) noexcept;

template<>
void PushNativeInternal(lua_State* L,
		lua_CFunction any) noexcept;

//不使用c++17的if constexpr，改用模板特化
//template<typename T>
//T ToNativeInternal(lua_State* L,
//		int idx)
//{
//	if constexpr (std::is_same<T, bool>::value)
//	{
//		if(lua_type(L, idx) != LUA_TBOOLEAN)
//			throw LuaException("can't cast to boolean");
//		return lua_toboolean(L, idx);
//	}
//	else if constexpr (std::is_integral<T>::value)
//	{
//		if(lua_type(L, idx) != LUA_TNUMBER)
//			throw LuaException("can't cast to integer");
//		return lua_tointeger(L, idx);
//	}
//	else if constexpr (std::is_same<T, std::string>::value)
//	{
//		if(lua_type(L, idx) != LUA_TSTRING)
//			throw LuaException("can't cast to std::string");
//		return std::string(lua_tostring(L, idx));
//	}
//	else if constexpr (std::is_floating_point<T>::value)
//	{
//		if(lua_type(L, idx) != LUA_TNUMBER)
//			throw LuaException("can't cast to float point");
//		return lua_tonumber(L, idx);
//	}
//	else if constexpr (std::is_pointer<T>::value)
//	{
//		using Type = std::remove_volatile<std::remove_pointer<T>::type>::type;
//		if constexpr (std::is_same<Type, const char>::value)
//		{
//			if(lua_type(L, idx) != LUA_TSTRING)
//				throw LuaException("can't cast to const char*");
//			return lua_tostring(L, idx);
//		}
//		else if constexpr (std::is_same<T, lua_CFunction>::value)
//		{
//			if(lua_type(L, idx) != LUA_TFUNCTION)
//				throw LuaException("can't cast to lua_CFunction");
//			return lua_tocfunction(L, idx);
//		}
//		else
//		{
//			int type = lua_type(L, idx);
//			if(type != LUA_TUSERDATA && type != LUA_TLIGHTUSERDATA)
//				throw LuaException("can't cast to userdata");
//			return (T)lua_touserdata(L, idx);
//		}
//	}
//}

template<typename T>
T ToNativeInternal(lua_State* L,
		int idx) noexcept
{
	static_assert(std::is_pointer<T>::value, "T is not pointer type");

	void* result = lua_touserdata(L, idx);
	if (result == nullptr)
		return nullptr;
	return (T)result;
}

template<>
bool ToNativeInternal(lua_State* L,
		int idx) noexcept;

template<>
int ToNativeInternal(lua_State* L,
		int idx) noexcept;

template<>
long ToNativeInternal(lua_State* L,
	int idx) noexcept;

template<>
std::string ToNativeInternal(lua_State* L,
		int idx) noexcept;

template<>
double ToNativeInternal(lua_State* L,
		int idx) noexcept;

template<>
float ToNativeInternal(lua_State* L,
		int idx) noexcept;

template<>
const char* ToNativeInternal(lua_State* L,
		int idx) noexcept;

template<>
lua_CFunction ToNativeInternal(lua_State* L,
		int idx) noexcept;

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

template<typename T, typename RetType, typename... ArgTypes, size_t... Integers>
RetType HelpCallObjectFunctionInternal(T* pointer,
	RetType(T::* func)(ArgTypes...) const,
	std::tuple<ArgTypes...>& tuple,
	std::index_sequence<Integers...>&&)
{
	return (pointer->*func)(std::get<Integers>(tuple)...);
}

template<typename RetType, typename... ArgTypes, size_t... Integers>
RetType HelpCallFunctionInternal(
		RetType(* function)(ArgTypes...),
		std::tuple<ArgTypes...>& tuple,
		std::index_sequence<Integers...>&&)
{
	return function(std::get<Integers>(tuple)...);
}

void CallLuaFunctionInternal(lua_State* L,
		int argCount,
		int retCount);

#endif