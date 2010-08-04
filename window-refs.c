
#include <lua.h>
#include <lauxlib.h>

#include "window-refs.h"

static char const * const window_ref_table_name = "GSL.windows";

void
window_ref_prepare (lua_State *L)
{
  lua_newtable (L);
  lua_setfield (L, LUA_REGISTRYINDEX, window_ref_table_name);
}

size_t 
window_ref_add(lua_State *L, int index)
{
  size_t n;

  lua_getfield (L, LUA_REGISTRYINDEX, window_ref_table_name);

  n = lua_objlen (L, -1);

  lua_pushvalue (L, index);
  lua_rawseti (L, -2, n+1);
  lua_pop (L, 1);

  return n+1;
}

void
window_ref_remove(lua_State *L, int id)
{
  lua_getfield (L, LUA_REGISTRYINDEX, window_ref_table_name);
  lua_pushnil (L);
  lua_rawseti (L, -2, id);
  lua_pop (L, 1);
}
