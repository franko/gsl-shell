
#include <lua.h>
#include <lauxlib.h>

#include "window-refs.h"
#include "canvas-window.h"

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
window_ref_remove (lua_State *L, int id)
{
  lua_getfield (L, LUA_REGISTRYINDEX, window_ref_table_name);
  lua_pushnil (L);
  lua_rawseti (L, -2, id);
  lua_pop (L, 1);
}

void
window_ref_close_all (lua_State *L)
{
  lua_getfield (L, LUA_REGISTRYINDEX, window_ref_table_name);

  lua_pushnil (L);  /* first key */
  while (lua_next(L, -2) != 0) 
    {
      lua_pushcfunction (L, canvas_window_close_protected);
      lua_insert (L, -2);
      lua_call (L, 1, 0);
    }

  lua_pop (L, 1);
}

int
window_ref_count (lua_State *L)
{
  int count = 0;

  lua_getfield (L, LUA_REGISTRYINDEX, window_ref_table_name);

  lua_pushnil (L);  /* first key */
  while (lua_next(L, -2) != 0) 
    {
      lua_pop (L, 1);
      count ++;
    }

  lua_pop (L, 1);

  return count;
}
