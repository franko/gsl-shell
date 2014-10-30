#include <lua.h>
#include <lauxlib.h>

#include "defs.h"

__BEGIN_DECLS

void register_properties_index(lua_State* L, const luaL_Reg methods[],
	const luaL_Reg getters[], const luaL_Reg setters[]);

__END_DECLS