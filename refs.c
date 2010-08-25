
#include "lua.h"
#include "lauxlib.h"

#include "refs.h"

void
register_ref_table (lua_State *L, const char *table_name)
{
  lua_newtable (L);

  /* the metatable to define it as a weak table */
  lua_newtable (L);
  lua_pushstring (L, "k");
  lua_setfield (L, -2, "__mode");
  lua_setmetatable (L, -2);

  lua_setfield (L, LUA_REGISTRYINDEX, table_name);
}
