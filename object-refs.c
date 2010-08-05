
#include <lua.h>
#include <lauxlib.h>

#include "object-refs.h"

static char const * const object_ref_table_name = "GSL.objref";

void
object_ref_prepare (lua_State *L)
{
  lua_newtable (L);

  /* the metatable to define it as a weak table */
  lua_newtable (L);
  lua_pushstring (L, "k");
  lua_setfield (L, -2, "__mode");
  lua_setmetatable (L, -2);

  lua_setfield (L, LUA_REGISTRYINDEX, object_ref_table_name);
}

void
object_ref_add (lua_State *L, int key_index, int val_index)
{
  size_t n;

  lua_getfield (L, LUA_REGISTRYINDEX, object_ref_table_name);
  lua_pushvalue (L, key_index);
  lua_pushvalue (L, key_index);
  lua_rawget (L, -3);

  if (lua_isnil (L, -1))
    {
      lua_pop (L, 1);
      lua_newtable (L);
    }

  n = lua_objlen (L, -1);

  lua_pushvalue (L, val_index);
  lua_rawseti (L, -2, n + 1);

  lua_rawset (L, -3);
  lua_pop (L, 1);
}
