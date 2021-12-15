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

#define REGISTER_CLASS(ClassName, L) ClassName##LuaWrapper::Register(L)

#define BEGIN_DECLARE_LUA_WRAPPER(ClassName, StaticMethodNum, StaticFieldNum, MethodNum, FieldNum)                                                                       \
    class ClassName##LuaWrapper                                                                                                                                          \
    {                                                                                                                                                                    \
        using T = ClassName;                                                                                                                                             \
                                                                                                                                                                         \
    public:                                                                                                                                                              \
        ClassName##LuaWrapper() = delete;                                                                                                                                \
        static void Register(lua_State *L)                                                                                                                               \
        {                                                                                                                                                                \
            int pop = 2;                                                                                                                                                 \
            int classMethodIdx = 0;                                                                                                                                      \
            if (StaticMethodNum > 0)                                                                                                                                     \
            {                                                                                                                                                            \
                CreateGlobalTable(L, GetClassMethodName(), 0, StaticMethodNum);                                                                                          \
                classMethodIdx = lua_gettop(L);                                                                                                                          \
                ++pop;                                                                                                                                                   \
            }                                                                                                                                                            \
            int classSetterIdx = 0;                                                                                                                                      \
            if (StaticFieldNum > 0)                                                                                                                                      \
            {                                                                                                                                                            \
                CreateGlobalTable(L, GetClassSetterName(), 0, StaticMethodNum);                                                                                          \
                classSetterIdx = lua_gettop(L);                                                                                                                          \
                ++pop;                                                                                                                                                   \
            }                                                                                                                                                            \
            int classGetterIdx = 0;                                                                                                                                      \
            if (StaticFieldNum > 0)                                                                                                                                      \
            {                                                                                                                                                            \
                CreateGlobalTable(L, GetClassGetterName(), 0, StaticMethodNum);                                                                                          \
                classGetterIdx = lua_gettop(L);                                                                                                                          \
                ++pop;                                                                                                                                                   \
            }                                                                                                                                                            \
            int methodIdx = 0;                                                                                                                                           \
            if (MethodNum > 0)                                                                                                                                           \
            {                                                                                                                                                            \
                CreateGlobalTable(L, GetObjectMethodName(), 0, StaticFieldNum);                                                                                          \
                methodIdx = lua_gettop(L);                                                                                                                               \
                ++pop;                                                                                                                                                   \
            }                                                                                                                                                            \
            int setterIdx = 0;                                                                                                                                           \
            if (FieldNum > 0)                                                                                                                                            \
            {                                                                                                                                                            \
                CreateGlobalTable(L, GetObjectSetterName(), 0, MethodNum);                                                                                               \
                setterIdx = lua_gettop(L);                                                                                                                               \
                ++pop;                                                                                                                                                   \
            }                                                                                                                                                            \
            int getterIdx = 0;                                                                                                                                           \
            if (FieldNum > 0)                                                                                                                                            \
            {                                                                                                                                                            \
                CreateGlobalTable(L, GetObjectGetterName(), 0, FieldNum);                                                                                                \
                getterIdx = lua_gettop(L);                                                                                                                               \
                ++pop;                                                                                                                                                   \
            }                                                                                                                                                            \
            CreateGlobalTable(L, GetObjectMetaName(), 0, 3);                                                                                                             \
            lua_pushstring(L, "__index");                                                                                                                                \
            lua_pushcfunction(L, IndexMethod);                                                                                                                           \
            lua_rawset(L, -3);                                                                                                                                           \
            lua_pushstring(L, "__newindex");                                                                                                                             \
            lua_pushcfunction(L, NewIndexMethod);                                                                                                                        \
            lua_rawset(L, -3);                                                                                                                                           \
            lua_pushstring(L, "__gc");                                                                                                                                   \
            lua_pushcfunction(L, DestroyObject);                                                                                                                         \
            lua_rawset(L, -3);                                                                                                                                           \
            CreateGlobalTable(L, GetClassName(), 0, 1);                                                                                                                  \
            int classIdx = lua_gettop(L);                                                                                                                                \
            lua_pushstring(L, "New");                                                                                                                                    \
            lua_pushcfunction(L, CreateObject);                                                                                                                          \
            lua_rawset(L, classIdx);                                                                                                                                     \
            lua_pushvalue(L, classIdx);                                                                                                                                  \
            lua_setglobal(L, #ClassName);                                                                                                                                \
            CreateGlobalTable(L, GetClassMetaName(), 0, 2);                                                                                                              \
            lua_pushstring(L, "__index");                                                                                                                                \
            lua_pushcfunction(L, StaticIndexMethod);                                                                                                                     \
            lua_rawset(L, -3);                                                                                                                                           \
            lua_pushstring(L, "__newindex");                                                                                                                             \
            lua_pushcfunction(L, StaticNewIndexMethod);                                                                                                                  \
            lua_rawset(L, -3);                                                                                                                                           \
            lua_setmetatable(L, classIdx);                                                                                                                               \
            int memberIdx = 0;                                                                                                                                           \
            MemberType *members = GetMembers();                                                                                                                          \
            MemberType member = *(members + memberIdx);                                                                                                                  \
            while (std::get<0>(member) != nullptr)                                                                                                                       \
            {                                                                                                                                                            \
                if (std::get<3>(member) == LUA_WRAPPER_STATIC)                                                                                                           \
                {                                                                                                                                                        \
                    int tableType = std::get<2>(member);                                                                                                                 \
                    int tableIdx = tableType == LUA_WRAPPER_MEMBER_METHOD ? classMethodIdx : (tableType == LUA_WRAPPER_MEMBER_GETTER ? classGetterIdx : classSetterIdx); \
                    if (tableIdx != 0)                                                                                                                                   \
                    {                                                                                                                                                    \
                        lua_pushstring(L, std::get<0>(member));                                                                                                          \
                        lua_pushcfunction(L, std::get<1>(member));                                                                                                       \
                        lua_rawset(L, tableIdx);                                                                                                                         \
                    }                                                                                                                                                    \
                }                                                                                                                                                        \
                else if (std::get<3>(member) == LUA_WRAPPER_OBJECT)                                                                                                      \
                {                                                                                                                                                        \
                    int tableType = std::get<2>(member);                                                                                                                 \
                    int tableIdx = tableType == LUA_WRAPPER_MEMBER_METHOD ? methodIdx : (tableType == LUA_WRAPPER_MEMBER_GETTER ? getterIdx : setterIdx);                \
                    if (tableIdx != 0)                                                                                                                                   \
                    {                                                                                                                                                    \
                        lua_pushstring(L, std::get<0>(member));                                                                                                          \
                        lua_pushcfunction(L, std::get<1>(member));                                                                                                       \
                        lua_rawset(L, tableIdx);                                                                                                                         \
                    }                                                                                                                                                    \
                }                                                                                                                                                        \
                member = *(members + (++memberIdx));                                                                                                                     \
            }                                                                                                                                                            \
            lua_pop(L, pop);                                                                                                                                             \
        }                                                                                                                                                                \
                                                                                                                                                                         \
    private:                                                                                                                                                             \
        static const char *GetClassName()                                                                                                                                \
        {                                                                                                                                                                \
            return "class " #ClassName;                                                                                                                                  \
        }                                                                                                                                                                \
        static const char *GetClassMetaName()                                                                                                                            \
        {                                                                                                                                                                \
            return "class " #ClassName "_StaticMeta";                                                                                                                    \
        }                                                                                                                                                                \
        static const char *GetClassMethodName()                                                                                                                          \
        {                                                                                                                                                                \
            return "class " #ClassName "_StaticMethod";                                                                                                                  \
        }                                                                                                                                                                \
        static const char *GetClassGetterName()                                                                                                                          \
        {                                                                                                                                                                \
            return "class " #ClassName "_StaticGetter";                                                                                                                  \
        }                                                                                                                                                                \
        static const char *GetClassSetterName()                                                                                                                          \
        {                                                                                                                                                                \
            return "class " #ClassName "_StaticSetter";                                                                                                                  \
        }                                                                                                                                                                \
        static const char *GetObjectMetaName()                                                                                                                           \
        {                                                                                                                                                                \
            return "class " #ClassName "_Meta";                                                                                                                          \
        }                                                                                                                                                                \
        static const char *GetObjectMethodName()                                                                                                                         \
        {                                                                                                                                                                \
            return "class " #ClassName "_Method";                                                                                                                        \
        }                                                                                                                                                                \
        static const char *GetObjectGetterName()                                                                                                                         \
        {                                                                                                                                                                \
            return "class " #ClassName "_Getter";                                                                                                                        \
        }                                                                                                                                                                \
        static const char *GetObjectSetterName()                                                                                                                         \
        {                                                                                                                                                                \
            return "class " #ClassName "_Setter";                                                                                                                        \
        }

#define END_DECLARE_LUA_WRAPPER()                                                  \
    static int StaticIndexMethod(lua_State *L)                                     \
    {                                                                              \
        const char *memberName = ToNative<const char *>(L, 2);                     \
        if (lua_getfield(L, LUA_REGISTRYINDEX, GetClassMethodName()) != LUA_TNIL)  \
        {                                                                          \
            if (lua_getfield(L, -1, memberName) != LUA_TNIL)                       \
            {                                                                      \
                lua_remove(L, -2);                                                 \
                return 1;                                                          \
            }                                                                      \
            lua_pop(L, 1);                                                         \
        }                                                                          \
        lua_pop(L, 1);                                                             \
        if (lua_getfield(L, LUA_REGISTRYINDEX, GetClassGetterName()) != LUA_TNIL)  \
        {                                                                          \
            if (lua_getfield(L, -1, memberName) != LUA_TNIL)                       \
            {                                                                      \
                lua_CFunction func = ToNative<lua_CFunction>(L, -1);               \
                lua_pop(L, 2);                                                     \
                return func(L);                                                    \
            }                                                                      \
            lua_pop(L, 1);                                                         \
        }                                                                          \
        lua_pop(L, 1);                                                             \
        return 0;                                                                  \
    }                                                                              \
    static int StaticNewIndexMethod(lua_State *L)                                  \
    {                                                                              \
        const char *memberName = ToNative<const char *>(L, 2);                     \
        if (lua_getfield(L, LUA_REGISTRYINDEX, GetClassSetterName()) != LUA_TNIL)  \
        {                                                                          \
            if (lua_getfield(L, -1, memberName) != LUA_TNIL)                       \
            {                                                                      \
                lua_CFunction func = ToNative<lua_CFunction>(L, -1);               \
                lua_pop(L, 2);                                                     \
                return func(L);                                                    \
            }                                                                      \
            lua_pop(L, 1);                                                         \
        }                                                                          \
        lua_pop(L, 1);                                                             \
        luaL_error(L, "forbid to add new field %s", memberName);                   \
        return 0;                                                                  \
    }                                                                              \
    static int IndexMethod(lua_State *L)                                           \
    {                                                                              \
        const char *memberName = ToNative<const char *>(L, 2);                     \
        if (lua_getfield(L, LUA_REGISTRYINDEX, GetObjectMethodName()) != LUA_TNIL) \
        {                                                                          \
            if (lua_getfield(L, -1, memberName) != LUA_TNIL)                       \
            {                                                                      \
                lua_remove(L, -2);                                                 \
                return 1;                                                          \
            }                                                                      \
            lua_pop(L, 1);                                                         \
        }                                                                          \
        lua_pop(L, 1);                                                             \
        if (lua_getfield(L, LUA_REGISTRYINDEX, GetObjectGetterName()) != LUA_TNIL) \
        {                                                                          \
            if (lua_getfield(L, -1, memberName) != LUA_TNIL)                       \
            {                                                                      \
                lua_CFunction func = ToNative<lua_CFunction>(L, -1);               \
                lua_pop(L, 2);                                                     \
                return func(L);                                                    \
            }                                                                      \
            lua_pop(L, 1);                                                         \
        }                                                                          \
        lua_pop(L, 1);                                                             \
        return 0;                                                                  \
    }                                                                              \
    static int NewIndexMethod(lua_State *L)                                        \
    {                                                                              \
        const char *memberName = ToNative<const char *>(L, 2);                     \
        if (lua_getfield(L, LUA_REGISTRYINDEX, GetObjectSetterName()) != LUA_TNIL) \
        {                                                                          \
            if (lua_getfield(L, -1, memberName) != LUA_TNIL)                       \
            {                                                                      \
                lua_CFunction func = ToNative<lua_CFunction>(L, -1);               \
                lua_pop(L, 2);                                                     \
                return func(L);                                                    \
            }                                                                      \
            lua_pop(L, 1);                                                         \
        }                                                                          \
        lua_pop(L, 1);                                                             \
        luaL_error(L, "forbid to add new field %s", memberName);                   \
        return 0;                                                                  \
    }                                                                              \
    }                                                                              \
    ;

#define DECLARE_CONSTRUCTOR(...)                                 \
    static int CreateObject(lua_State *L)                        \
    {                                                            \
        try                                                      \
        {                                                        \
            CreateUserData<T, __VA_ARGS__>(L);                   \
        }                                                        \
        catch (LuaException & e)                                 \
        {                                                        \
            luaL_error(L, e.what());                             \
            return 0;                                            \
        }                                                        \
        lua_getfield(L, LUA_REGISTRYINDEX, GetObjectMetaName()); \
        lua_setmetatable(L, -2);                                 \
        return 1;                                                \
    }

#define DECLARE_EMPTY_CONSTRUCTOR()    \
    static int CreateObject(lua_State *L) \
    {                                     \
        return 0;                         \
    }

#define DECLARE_DEFAULT_CONSTRUCTOR()                            \
    static int CreateObject(lua_State *L)                        \
    {                                                            \
        CreateUserDataNoParam<T>(L);                             \
        lua_getfield(L, LUA_REGISTRYINDEX, GetObjectMetaName()); \
        lua_setmetatable(L, -2);                                 \
        return 1;                                                \
    }

#define DECLARE_DESTRUCTOR(Destructor)            \
    static int DestroyObject(lua_State *L)        \
    {                                             \
        T **pointer = (T **)lua_touserdata(L, 1); \
        Destructor(*pointer);                     \
        return 1;                                 \
    }

#define DECLARE_EMPTY_DESTRUCTOR()           \
    static int DestroyObject(lua_State *L)   \
    {                                        \
        return 0;                            \
    }

#define DECLARE_DEFAULT_DESTRUCTOR()              \
    static int DestroyObject(lua_State *L)        \
    {                                             \
        T **pointer = (T **)lua_touserdata(L, 1); \
        delete *pointer;                          \
        return 1;                                 \
    }

#define BEGIN_DECLARE_MEMBER()                                                   \
    using MemberType = std::tuple<const char *, int (*)(lua_State *), int, int>; \
    static MemberType *GetMembers()                                              \
    {                                                                            \
        static MemberType tmembers[] = {

#define END_DECLARE_MEMBER()   \
    {                          \
        nullptr, nullptr, 0, 0 \
    }                          \
    }                          \
    ;                          \
    return tmembers;           \
    }

#define DECLARE_STATIC_METHOD_AS(MethodName, Method, RetType, ...)        \
    {MethodName, [](lua_State *L) {                                       \
         int ret = 0;                                                     \
         try                                                              \
         {                                                                \
             ret = HelpCallFunction<RetType, __VA_ARGS__>(L, &T::Method); \
         }                                                                \
         catch (LuaException & e)                                         \
         {                                                                \
             luaL_error(L, e.what());                                     \
         }                                                                \
         return ret;                                                      \
     },                                                                   \
     LUA_WRAPPER_MEMBER_METHOD, LUA_WRAPPER_STATIC},

#define DECLARE_STATIC_METHOD(Method, RetType, ...) \
    DECLARE_STATIC_METHOD_AS(#Method, Method, RetType, __VA_ARGS__)

#define DECLARE_STATIC_FIELD_AS(FieldName, Field)            \
    {FieldName, [](lua_State *L) {                           \
         try                                                 \
         {                                                   \
             T::Field = ToNative<decltype(T::Field)>(L, -1); \
         }                                                   \
         catch (LuaException & e)                            \
         {                                                   \
             luaL_error(L, e.what());                        \
         }                                                   \
         return 0;                                           \
     },                                                      \
     LUA_WRAPPER_MEMBER_SETTER, LUA_WRAPPER_STATIC},         \
        {FieldName, [](lua_State *L) {                       \
             PushNative(L, T::Field);                        \
             return 1;                                       \
         },                                                  \
         LUA_WRAPPER_MEMBER_GETTER, LUA_WRAPPER_STATIC},

#define DECLARE_STATIC_FIELD(Field) \
    DECLARE_STATIC_FIELD_AS(#Field, Field)

#define DECLARE_METHOD_AS(MethodName, Method, RetType, ...)                        \
    {MethodName, [](lua_State *L) {                                                \
         int ret = 0;                                                              \
         try                                                                       \
         {                                                                         \
             ret = HelpCallObjectFunction<T, RetType, __VA_ARGS__>(L, &T::Method); \
         }                                                                         \
         catch (LuaException & e)                                                  \
         {                                                                         \
             luaL_error(L, e.what());                                              \
         }                                                                         \
         return ret;                                                               \
     },                                                                            \
     LUA_WRAPPER_MEMBER_METHOD, LUA_WRAPPER_OBJECT},

#define DECLARE_METHOD(Method, RetType, ...) \
    DECLARE_METHOD_AS(#Method, Method, RetType, __VA_ARGS__)

#define DECLARE_FIELD_AS(FieldName, Field)              \
    {FieldName, [](lua_State *L) {                      \
         T **userData = (T **)lua_touserdata(L, 1);     \
         try                                            \
         {                                              \
             using Type = decltype(T::Field);           \
             (*userData)->Field = ToNative<Type>(L, 2); \
         }                                              \
         catch (LuaException & e)                       \
         {                                              \
             luaL_error(L, e.what());                   \
         }                                              \
         return 0;                                      \
     },                                                 \
     LUA_WRAPPER_MEMBER_SETTER, LUA_WRAPPER_OBJECT},    \
        {FieldName, [](lua_State *L) {                  \
             T **userData = (T **)lua_touserdata(L, 1); \
             PushNative(L, (*userData)->Field);         \
             return 1;                                  \
         },                                             \
         LUA_WRAPPER_MEMBER_GETTER, LUA_WRAPPER_OBJECT},

#define DECLARE_FIELD(Field) \
    DECLARE_FIELD_AS(#Field, Field)

template <typename T>
class HelloLua
{
    friend class HelloLuaHelper;

public:
    HelloLua<T> *BeginClass(int staticMethodNum,
                            int staticFieldNum,
                            int objectMethodNum,
                            int objectFieldNum)
    {
        char name[MAX_OBJECT_META_NAME_LEN];
        //class
        CreateGlobalTable(_luaState, GetClassName(), 0, 1);
        PushNatives<const char *, lua_CFunction>(_luaState, {"New", CreateObject});
        lua_rawset(_luaState, -3);
        //static meta
        GetStaticMetaName(name, MAX_OBJECT_META_NAME_LEN);
        CreateGlobalTable(_luaState, name, 0, 2);
        PushNatives<const char *, lua_CFunction>(_luaState, {"__index", StaticIndexMethod});
        lua_rawset(_luaState, -3);
        PushNatives<const char *, lua_CFunction>(_luaState, {"__newindex", StaticNewIndexMethod});
        lua_rawset(_luaState, -3);
        lua_setmetatable(_luaState, -2);
        //global class
        lua_setglobal(_luaState, GetClassName());

        if (staticMethodNum > 0)
        {
            GetStaticMethodName(name, MAX_OBJECT_META_NAME_LEN);
            CreateGlobalTable(_luaState, name, 0, staticMethodNum);
            lua_pop(_luaState, 1);
        }
        if (staticFieldNum > 0)
        {
            GetStaticSetterName(name, MAX_OBJECT_META_NAME_LEN);
            CreateGlobalTable(_luaState, name, 0, staticFieldNum);
            lua_pop(_luaState, 1);
        }
        if (staticFieldNum > 0)
        {
            GetStaticGetterName(name, MAX_OBJECT_META_NAME_LEN);
            CreateGlobalTable(_luaState, name, 0, staticFieldNum);
            lua_pop(_luaState, 1);
        }

        //object meta
        GetObjectMetaName(name, MAX_OBJECT_META_NAME_LEN);
        CreateGlobalTable(_luaState, name, 0, 3);
        PushNatives<const char *, lua_CFunction>(_luaState, {"__index", IndexMethod});
        lua_rawset(_luaState, -3);
        PushNatives<const char *, lua_CFunction>(_luaState, {"__newindex", NewIndexMethod});
        lua_rawset(_luaState, -3);
        lua_pop(_luaState, 1);

        if (objectMethodNum > 0)
        {
            GetObjectMethodName(name, MAX_OBJECT_META_NAME_LEN);
            CreateGlobalTable(_luaState, name, 0, objectMethodNum);
            lua_pop(_luaState, 1);
        }
        if (objectFieldNum > 0)
        {
            GetObjectGetterName(name, MAX_OBJECT_META_NAME_LEN);
            CreateGlobalTable(_luaState, name, 0, objectFieldNum);
            lua_pop(_luaState, 1);
        }
        if (objectFieldNum > 0)
        {
            GetObjectSetterName(name, MAX_OBJECT_META_NAME_LEN);
            CreateGlobalTable(_luaState, name, 0, objectFieldNum);
            lua_pop(_luaState, 1);
        }

        return this;
    }

    HelloLua<T> *AddDestructor(lua_CFunction destructor)
    {
        char name[MAX_OBJECT_META_NAME_LEN];
        GetObjectMetaName(name, MAX_OBJECT_META_NAME_LEN);
        if (lua_getfield(_luaState, LUA_REGISTRYINDEX, name) != LUA_TNIL)
        {
            PushNatives<const char *, lua_CFunction>(_luaState, {"__gc", destructor});
            lua_rawset(_luaState, -3);
        }
        lua_pop(_luaState, 1);
        return this;
    }

    HelloLua<T> *AddDefaultDestructor()
    {
        char name[MAX_OBJECT_META_NAME_LEN];
        GetObjectMetaName(name, MAX_OBJECT_META_NAME_LEN);
        if (lua_getfield(_luaState, LUA_REGISTRYINDEX, name) != LUA_TNIL)
        {
            PushNatives<const char *, lua_CFunction>(_luaState, {"__gc", [](lua_State *L)
                                                                 {
                                                                     T **userData = (T **)lua_touserdata(L, 1);
                                                                     delete (*userData);
                                                                     return 0;
                                                                 }});
            lua_rawset(_luaState, -3);
        }
        lua_pop(_luaState, 1);
        return this;
    }

    template <typename... ArgTypes>
    HelloLua<T> *AddConstructor()
    {
        _constructor = [](lua_State *L)
        {
            CreateUserData<T, ArgTypes...>(L);
            return 1;
        };
        return this;
    }

    HelloLua<T> *AddDefaultConstructor()
    {
        _constructor = [](lua_State *L)
        {
            CreateUserDataNoParam<T>(L);
            return 1;
        };
        return this;
    }

    template <typename RetType, typename... ArgTypes>
    HelloLua<T> *AddStaticMethod(const char *name, RetType (*func)(ArgTypes...))
    {
        char key[MAX_OBJECT_META_NAME_LEN];
        GetStaticMethodName(key, MAX_OBJECT_META_NAME_LEN);
        if (lua_getfield(_luaState, LUA_REGISTRYINDEX, key) != LUA_TNIL)
        {
            PushNatives<const char *, lua_CFunction>(_luaState, {name, [func](lua_State *L)
                                                                 {
                                                                     int ret = 0;
                                                                     try
                                                                     {
                                                                         ret = HelpCallFunction<RetType, ArgTypes...>(L, func);
                                                                     }
                                                                     catch (LuaException &e)
                                                                     {
                                                                         luaL_error(L, e.what());
                                                                     }
                                                                     return ret;
                                                                 }});
            lua_rawset(_luaState, -3);
        }
        lua_pop(_luaState, 1);
        return this;
    }

    template <typename RetType>
    HelloLua<T> *AddStaticField(const char *name, RetType *field);

    template <typename RetType, typename... ArgTypes>
    HelloLua<T> *AddMethod(const char *name, RetType (T::*func)(ArgTypes...));

    template <typename RetType>
    HelloLua<T> *AddField(const char *name, RetType(T::*field));

private:
    explicit HelloLua() : _luaState(nullptr)
    {
    }

    //field
    lua_State *_luaState;
    lua_CFunction _constructor;

    //static
    static int CreateObject(lua_State *L)
    {
        HelloLua<T> *helloLua = HelloLuaHelper::GetHelloLua<T>(L);
        if (helloLua->_constructor != nullptr)
            return helloLua->_constructor(L);

        return 0;
    }

    static int StaticIndexMethod(lua_State *L)
    {
    }

    static int StaticNewIndexMethod(lua_State *L)
    {
    }

    static int IndexMethod(lua_State *L)
    {
    }

    static int NewIndexMethod(lua_State *L)
    {
    }

    // method
    static const char *GetClassName()
    {
        return typeid(T).name();
    }

    static void GetStaticMetaName(char *buffer, size_t size)
    {
        snprintf(buffer, size, "%s_StaticMeta", GetClassName());
    }

    static void GetStaticMethodName(char *buffer, size_t size)
    {
        snprintf(buffer, size, "%s_StaticMethod", GetClassName());
    }

    static void GetStaticGetterName(char *buffer, size_t size)
    {
        snprintf(buffer, size, "%s_StaticGetter", GetClassName());
    }

    static void GetStaticSetterName(char *buffer, size_t size)
    {
        snprintf(buffer, size, "%s_StaticSetter", GetClassName());
    }

    static void GetObjectMetaName(char *buffer, size_t size)
    {
        snprintf(buffer, size, "%s_Meta", GetClassName());
    }

    static void GetObjectMethodName(char *buffer, size_t size)
    {
        snprintf(buffer, size, "%s_Method", GetClassName());
    }

    static void GetObjectGetterName(char *buffer, size_t size)
    {
        snprintf(buffer, size, "%s_Getter", GetClassName());
    }

    static void GetObjectSetterName(char *buffer, size_t size)
    {
        snprintf(buffer, size, "%s_Setter", GetClassName());
    }
};

class HelloLuaHelper
{
public:
    HelloLuaHelper() = delete;

    template <typename T>
    static HelloLua<T> *GetHelloLua(lua_State *L)
    {
        if (!L)
            return nullptr;

        static HelloLua<T> helloLua;

        helloLua._luaState = L;

        return &helloLua;
    }
};

#endif
