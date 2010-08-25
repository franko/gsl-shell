
#include <lua.h>
#include <lauxlib.h>

#include "object-index.h"

#warning OBJECT_PLOT is no longer used
static const char *table_name[] = {"GSL.reg.wins", "GSL.reg.plts"};

void
object_index_prepare (lua_State *L)
{
  lua_newtable (L);
  lua_setfield (L, LUA_REGISTRYINDEX, table_name[OBJECT_WINDOW]);

  lua_newtable (L);
  /* the metatable to define it as a weak table */
  lua_newtable (L);
  lua_pushstring (L, "v");
  lua_setfield (L, -2, "__mode");
  lua_setmetatable (L, -2);
  lua_setfield (L, LUA_REGISTRYINDEX, table_name[OBJECT_PLOT]);
}

int
object_index_add(lua_State *L, int obj_class, int index)
{
  int n;

  if (index < 0)
    index = lua_gettop (L) - (index+1);

  lua_getfield (L, LUA_REGISTRYINDEX, table_name[obj_class]);

  n = lua_objlen (L, -1);

  lua_pushvalue (L, index);
  lua_rawseti (L, -2, n+1);
  lua_pop (L, 1);

  return n+1;
}

void
object_index_get (lua_State *L, int obj_class, int id)
{
  lua_getfield (L, LUA_REGISTRYINDEX, table_name[obj_class]);
  lua_rawgeti (L, -1, id);
  lua_remove (L, -2);
}

void
object_index_remove (lua_State *L, int obj_class, int id)
{
  lua_getfield (L, LUA_REGISTRYINDEX, table_name[obj_class]);
  lua_pushnil (L);
  lua_rawseti (L, -2, id);
  lua_pop (L, 1);
}

void
object_index_apply_all (lua_State *L, int obj_class, lua_CFunction f)
{
  lua_getfield (L, LUA_REGISTRYINDEX, table_name[obj_class]);

  lua_pushnil (L);  /* first key */
  while (lua_next(L, -2) != 0) 
    {
      /*      lua_pushcfunction (L, canvas_window_close_protected); */
      lua_pushcfunction (L, f);
      lua_insert (L, -2);
      lua_call (L, 1, 0);
    }

  lua_pop (L, 1);
}

int
object_index_count (lua_State *L, int obj_class)
{
  int count = 0;

  lua_getfield (L, LUA_REGISTRYINDEX, table_name[obj_class]);

  lua_pushnil (L);  /* first key */
  while (lua_next(L, -2) != 0) 
    {
      lua_pop (L, 1);
      count ++;
    }

  lua_pop (L, 1);

  return count;
}
