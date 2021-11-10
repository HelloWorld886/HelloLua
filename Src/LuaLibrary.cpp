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
