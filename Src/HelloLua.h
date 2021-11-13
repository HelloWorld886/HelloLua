//
// Created by musmusliu on 2021/10/27.
//

#ifndef HELLO_LUA_H_
#define HELLO_LUA_H_

#include "LuaLibrary.h"

#define LUA_WRAPPER_MEMBER_METHOD 0
#define LUA_WRAPPER_MEMBER_GETTER 1
#define LUA_WRAPPER_MEMBER_SETTER 2
#define LUA_WRAPPER_STATIC 0
#define LUA_WRAPPER_OBJECT 1

#define REGISTER_CLASS(ClassName, L) ClassName##LuaWrapper<ClassName>::RegisterClass(L)

#define BEGIN_DECLARE_LUA_WRAPPER(ClassName, StaticMethodNum, StaticFieldNum, MethodNum, FieldNum) \
template<typename T>                                                                                  \
class ClassName##LuaWrapper                                                                            \
{                                                                                                   \
public:                                                                                                   \
    static void RegisterClass(lua_State* L)                                                                  \
    {                                                                                                        \
        static ClassName##LuaWrapper luaWrapper;                                                             \
        luaWrapper.Register(L);                                                                \
    }                                                                                        \
private:                                                                                        \
    constexpr static int MemberNum = StaticMethodNum + StaticFieldNum*2 + MethodNum + FieldNum*2;    \
    ClassName##LuaWrapper(){}                                                            			\
    void Register(lua_State* L)                                                                    \
    {                                                                                            \
        int pop = 2;                                                                            \
        int classMethodIdx = 0;                                                                \
        if (StaticMethodNum > 0)                                                                \
        {                                                                                        \
            CreateGlobalTable(L, GetClassMethodName(), 0, StaticMethodNum);                        \
            classMethodIdx = lua_gettop(L);                                                    \
            ++pop;                                                                                \
        }                                                                                        \
        int classSetterIdx = 0;                                                                \
        if(StaticMethodNum > 0)                                                                    \
        {                                                                                        \
            CreateGlobalTable(L, GetClassSetterName(), 0, StaticMethodNum);                    \
            classSetterIdx = lua_gettop(L);                                                    \
            ++pop;                                                                                \
        }                                                                                        \
        int classGetterIdx = 0;                                                                \
        if(StaticMethodNum > 0)                                                                \
        {                                                                                        \
            CreateGlobalTable(L, GetClassGetterName() , 0, StaticMethodNum);                    \
            classGetterIdx = lua_gettop(L);                                                    \
            ++pop;                                                                                \
        }                                                                                        \
        int methodIdx = 0;                                                                        \
        if (StaticFieldNum > 0)                                                                \
        {                                                                                        \
            CreateGlobalTable(L, GetObjectMethodName(), 0, StaticFieldNum);                    \
            methodIdx = lua_gettop(L);                                                            \
            ++pop;                                                                                \
        }                                                                                        \
        int setterIdx = 0;                                                                        \
        if(MethodNum > 0)                                                                        \
        {                                                                                        \
            CreateGlobalTable(L, GetObjectSetterName(), 0, MethodNum);                            \
            setterIdx = lua_gettop(L);                                                            \
            ++pop;                                                                                \
        }                                                                                        \
        int getterIdx = 0;                                                                        \
        if(FieldNum > 0)                                                                        \
        {                                                                                        \
            CreateGlobalTable(L, GetObjectGetterName(), 0, FieldNum);                            \
            getterIdx = lua_gettop(L);                                                            \
            ++pop;                                                                                \
        }                                                                                        \
        CreateGlobalTable(L, GetObjectMetaName(), 0, 3);                                        \
        lua_pushstring(L, "__index");                                                            \
        lua_pushcfunction(L, IndexMethod);                                                        \
        lua_rawset(L, -3);                                                                        \
        lua_pushstring(L, "__newindex");                                                        \
        lua_pushcfunction(L, NewIndexMethod);                                                    \
        lua_rawset(L, -3);                                                                        \
        lua_pushstring(L, "__gc");                                                                \
        lua_pushcfunction(L, DestroyObject);                                                    \
        lua_rawset(L, -3);                                                                        \
        CreateGlobalTable(L, GetClassName(), 0, 1);                                            \
        int classIdx = lua_gettop(L);                                                            \
        lua_pushstring(L, "New");                                                                \
        lua_pushcfunction(L, CreateObject);                                                    \
        lua_rawset(L, classIdx);                                                                \
        lua_pushvalue(L, classIdx);                                                             \
        lua_setglobal(L, #ClassName);                                                            \
        CreateGlobalTable(L, GetClassMetaName(), 0, 2);                                        \
        lua_pushstring(L, "__index");                                                            \
        lua_pushcfunction(L, StaticIndexMethod);                                                \
        lua_rawset(L, -3);                                                                        \
        lua_pushstring(L, "__newindex");                                                        \
        lua_pushcfunction(L, StaticNewIndexMethod);                                            \
        lua_rawset(L, -3);                                                                        \
        lua_setmetatable(L, classIdx);                                                            \
        int memberIdx = 0;                                                                      \
        while(memberIdx < MemberNum && std::get<0>(_members[memberIdx]) != nullptr)    \
        {                                                                                       \
            if(std::get<3>(_members[memberIdx]) == LUA_WRAPPER_STATIC)                           \
            {                                                                                    \
                int tableType = std::get<2>(_members[memberIdx]);                                \
                int tableIdx = tableType == LUA_WRAPPER_MEMBER_METHOD ? classMethodIdx :        \
                    (tableType == LUA_WRAPPER_MEMBER_GETTER ? classGetterIdx : classSetterIdx); \
                if(tableIdx != 0)                                                               \
                {                                                                               \
                    lua_pushstring(L, std::get<0>(_members[memberIdx]));                            \
                    lua_pushcfunction(L, std::get<1>(_members[memberIdx]));                         \
                    lua_rawset(L, tableIdx);                                                    \
                }                                                                                \
            }                                                                                   \
            else if(std::get<3>(_members[memberIdx]) == LUA_WRAPPER_OBJECT)                      \
            {                                                                                   \
                int tableType = std::get<2>(_members[memberIdx]);                                \
                int tableIdx = tableType == LUA_WRAPPER_MEMBER_METHOD ? methodIdx :             \
                    (tableType == LUA_WRAPPER_MEMBER_GETTER ? getterIdx : setterIdx);           \
                if(tableIdx != 0)                                                               \
                {                                                                               \
                    lua_pushstring(L, std::get<0>(_members[memberIdx]));                            \
                    lua_pushcfunction(L, std::get<1>(_members[memberIdx]));                         \
                    lua_rawset(L, tableIdx);                                                    \
                }                                                                                \
            }                                                                                    \
            ++memberIdx;                                                                        \
        }                                                                                       \
        lua_pop(L, pop);                                                                        \
    }                                                                                            \
    static const char* GetClassName()                                                            \
    {                                                                                            \
        return "class "#ClassName;                                                                 \
    }                                                                                            \
    static const char* GetClassMetaName()                                                        \
    {                                                                                            \
        return "class "#ClassName"_StaticMeta";                                                            \
    }                                                                                            \
    static const char* GetClassMethodName()                                                        \
    {                                                                                            \
        return "class "#ClassName"_StaticMethod";                                                        \
    }                                                                                            \
    static const char* GetClassGetterName()                                                        \
    {                                                                                            \
        return "class "#ClassName"_StaticGetter";                                                        \
    }                                                                                            \
    static const char* GetClassSetterName()                                                        \
    {                                                                                            \
        return "class "#ClassName"_StaticSetter";                                                        \
    }                                                                                            \
    static const char* GetObjectMetaName()                                                        \
    {                                                                                            \
        return "class "#ClassName"_Meta";                                                                \
    }                                                                                            \
    static const char* GetObjectMethodName()                                                        \
    {                                                                                            \
        return "class "#ClassName"_Method";                                                                \
    }                                                                                            \
    static const char* GetObjectGetterName()                                                        \
    {                                                                                            \
        return "class "#ClassName"_Getter";                                                                \
    }                                                                                            \
    static const char* GetObjectSetterName()                                                        \
    {                                                                                            \
        return "class "#ClassName"_Setter";                                                                \
    }

#define END_DECLARE_LUA_WRAPPER()                                                    \
    static int StaticIndexMethod(lua_State* L)                                        \
    {                                                                                \
        const char* memberName = ToNative<const char*>(L, 2);                        \
        if(lua_getfield(L, LUA_REGISTRYINDEX, GetClassMethodName()) != LUA_TNIL)    \
        {                                                                            \
            if(lua_getfield(L, -1, memberName) != LUA_TNIL)                            \
            {                                                                        \
                lua_remove(L, -2);                                                    \
                return 1;                                                            \
            }                                                                        \
            lua_pop(L, 1);                                                            \
        }                                                                            \
        lua_pop(L, 1);                                                                \
        if(lua_getfield(L, LUA_REGISTRYINDEX, GetClassGetterName()) != LUA_TNIL)    \
        {                                                                            \
            if(lua_getfield(L, -1, memberName) != LUA_TNIL)                            \
            {                                                                        \
                lua_CFunction func = ToNative<lua_CFunction>(L, -1);                 \
                lua_pop(L,2);                                                        \
                return func(L);                                                      \
            }                                                                        \
            lua_pop(L, 1);                                                            \
        }                                                                            \
        lua_pop(L, 1);                                                                \
        return 0;                                                                    \
    }                                                                                \
    static int StaticNewIndexMethod(lua_State* L)                                    \
    {                                                                                \
        const char* memberName = ToNative<const char*>(L, 2);                        \
        if(lua_getfield(L, LUA_REGISTRYINDEX, GetClassSetterName()) != LUA_TNIL)    \
        {                                                                            \
            if(lua_getfield(L, -1, memberName) != LUA_TNIL)                            \
            {                                                                        \
                lua_CFunction func = ToNative<lua_CFunction>(L, -1);                 \
                lua_pop(L,2);                                                        \
                return func(L);                                                      \
            }                                                                        \
            lua_pop(L, 1);                                                            \
        }                                                                            \
        lua_pop(L, 1);                                                               \
        luaL_error(L, "forbid to add new field %s", memberName);                    \
        return 0;                                                                    \
    }                                                                                \
    static int IndexMethod(lua_State* L)                                            \
    {                                                                                \
        const char* memberName = ToNative<const char*>(L, 2);                        \
        if(lua_getfield(L, LUA_REGISTRYINDEX, GetObjectMethodName()) != LUA_TNIL)    \
        {                                                                            \
            if(lua_getfield(L, -1, memberName) != LUA_TNIL)                            \
            {                                                                        \
                lua_remove(L, -2);                                                  \
                return 1;                                                            \
            }                                                                        \
            lua_pop(L, 1);                                                            \
        }                                                                            \
        lua_pop(L, 1);                                                                \
        if(lua_getfield(L, LUA_REGISTRYINDEX, GetObjectGetterName()) != LUA_TNIL)    \
        {                                                                            \
            if(lua_getfield(L, -1, memberName) != LUA_TNIL)                            \
            {                                                                        \
                lua_CFunction func = ToNative<lua_CFunction>(L, -1);                 \
                lua_pop(L,2);                                                        \
                return func(L);                                                      \
            }                                                                        \
            lua_pop(L, 1);                                                            \
        }                                                                            \
        lua_pop(L, 1);                                                                \
        return 0;                                                                    \
    }                                                                                \
    static int NewIndexMethod(lua_State* L)                                            \
    {                                                                                \
        const char* memberName = ToNative<const char*>(L, 2);                        \
        if(lua_getfield(L, LUA_REGISTRYINDEX, GetObjectSetterName()) != LUA_TNIL)    \
        {                                                                            \
            if(lua_getfield(L, -1, memberName) != LUA_TNIL)                            \
            {                                                                        \
                lua_CFunction func = ToNative<lua_CFunction>(L, -1);                 \
                lua_pop(L,2);                                                        \
                return func(L);                                                      \
            }                                                                        \
            lua_pop(L, 1);                                                            \
        }                                                                            \
        lua_pop(L, 1);                                                                \
        luaL_error(L, "forbid to add new field %s", memberName);                     \
        return 0;                                                                    \
    }                                                                                \
};                                                                                    \

#define DECLARE_CONSTRUCTOR(...)                            	\
    static int CreateObject(lua_State* L)                    	\
    {                                                            \
        try{                                                	\
            CreateUserData<T, __VA_ARGS__>(L);                    \
        } catch(LuaException& e){                                \
            luaL_error(L, e.what());                            \
            return 0;                                            \
        }                                                        \
        lua_getfield(L,LUA_REGISTRYINDEX, GetObjectMetaName()); \
        lua_setmetatable(L, -2);                                \
        return 1;                                                \
    }                                                            \

#define DECLARE_DEFAULT_CONSTRUCTOR()                            \
    static int CreateObject(lua_State* L)                    	\
    {                                                            \
        CreateUserDataNoParam<T>(L);                            \
        lua_getfield(L,LUA_REGISTRYINDEX, GetObjectMetaName()); \
        lua_setmetatable(L, -2);                                \
        return 1;                                                \
    }                                                            \

#define DECLARE_DESTRUCTOR(Destructor)            \
    static int DestroyObject(lua_State* L)    \
    {                                            \
        T** pointer = (T**)lua_touserdata(L,1); \
        Destructor(*pointer);                    \
        return 1;                                \
    }                                            \

#define DECLARE_DEFAULT_DESTRUCTOR()            \
    static int DestroyObject(lua_State* L)    \
    {                                            \
        T** pointer = (T**)lua_touserdata(L, 1); \
        delete *pointer;                        \
        return 1;                                \
    }                                            \

#define BEGIN_DECLARE_MEMBER() \
    using MemberType = std::tuple<const char*, int (*)(lua_State*), int, int>;    \
    MemberType _members[MemberNum] = {                                            \

#define END_DECLARE_MEMBER()            \
    };                              	\

#define DECLARE_STATIC_METHOD_AS(MethodName, Method, RetType, ...)        \
        {MethodName, [](lua_State* L){                                    \
            int ret = 0;                                                           \
            try{                                                                   \
                ret = HelpCallFunction<RetType, __VA_ARGS__>                 \
                    (L, std::function<RetType(__VA_ARGS__)>(&T::Method));     \
            }catch(LuaException& e){                                            \
                luaL_error(L, e.what());  \
            }                                                                 \
            return ret;},                                                    \
        LUA_WRAPPER_MEMBER_METHOD, LUA_WRAPPER_STATIC},                        \

#define DECLARE_STATIC_METHOD(Method, RetType, ...)            \
    DECLARE_STATIC_METHOD_AS(#Method, Method, RetType, __VA_ARGS__)

#define DECLARE_STATIC_FIELD_AS(FieldName, Field)                \
        {FieldName, [](lua_State* L) {                        \
            try{                                           		\
                T::Field = ToNative<decltype(T::Field)>(L, -1); \
            }catch(LuaException& e){                            \
                luaL_error(L, e.what());                        \
            }                                                    \
            return 0; },                                        \
        LUA_WRAPPER_MEMBER_SETTER, LUA_WRAPPER_STATIC},            \
        {FieldName, [](lua_State* L) {                        \
            PushNative(L, T::Field);                            \
            return 1; },                                        \
        LUA_WRAPPER_MEMBER_GETTER, LUA_WRAPPER_STATIC},            \

#define DECLARE_STATIC_FIELD(Field)            \
        DECLARE_STATIC_FIELD_AS(#Field, Field)

#define DECLARE_METHOD_AS(MethodName, Method, RetType, ...)                                \
        {MethodName, [](lua_State* L){                                                \
            int ret = 0;                                                                \
            try{                                                                           \
                ret = HelpCallObjectFunction<T, RetType, __VA_ARGS__>(L, &T::Method);    \
            }catch(LuaException& e){                                                       \
                luaL_error(L, e.what());                                                 \
            }                                                                            \
            return ret;},                                                             \
        LUA_WRAPPER_MEMBER_METHOD, LUA_WRAPPER_OBJECT},                               \

#define DECLARE_METHOD(Method, RetType, ...)        \
    DECLARE_METHOD_AS(#Method, Method, RetType, __VA_ARGS__)

#define DECLARE_FIELD_AS(FieldName, Field)                \
        {FieldName, [](lua_State* L) {                    \
            T** userData = (T**)lua_touserdata(L, 1);     \
            try{                                           \
                using Type = decltype(T::Field);        \
                (*userData)->Field= ToNative<Type>(L, 2);    \
            }catch(LuaException& e){                     \
                luaL_error(L, e.what());                  \
            }                                             \
            return 0; },                               \
        LUA_WRAPPER_MEMBER_SETTER, LUA_WRAPPER_OBJECT},    \
        {FieldName, [](lua_State* L) {                    \
            T** userData = (T**)lua_touserdata(L, 1);     \
            PushNative(L,(*userData)->Field);        \
            return 1; },                                \
        LUA_WRAPPER_MEMBER_GETTER, LUA_WRAPPER_OBJECT},    \

#define DECLARE_FIELD(Field)        \
    DECLARE_FIELD_AS(#Field, Field)

#endif



