
#include <lua.h>
#include <lauxlib.h>

#include "refs.h"
#include "object-refs.h"

static char const * const object_ref_table_name = "GSL.objref";

void
object_ref_prepare (lua_State *L)
{
  register_ref_table (L, object_ref_table_name);
}

void
object_ref_add (lua_State *L, int key_index, int val_index)
{
  int ref;
  bool add_table = false;

  lua_getfield (L, LUA_REGISTRYINDEX, object_ref_table_name);
  lua_pushvalue (L, key_index);
  lua_rawget (L, -2);

  if (lua_isnil (L, -1))
    {
      lua_pop (L, 1);
      lua_newtable (L);

      lua_pushvalue (L, val_index);
      ref = 1;
      add_table = true;
    }
  else
    {
      lua_pushvalue (L, val_index);
      lua_pushvalue (L, val_index);
      lua_rawget (L, -3);

      ref = lua_tointeger (L, -1) + 1;

      lua_pop (L, 1);
    }

  lua_pushinteger (L, ref);
  lua_rawset (L, -3);

  if (add_table)
    {
      lua_pushvalue (L, key_index);
      lua_insert (L, -2);
      lua_rawset (L, -3);
      lua_pop (L, 1);
    }
  else
    lua_pop (L, 2);
}

void
object_ref_remove (lua_State *L, int key_index, int val_index)
{
  lua_getfield (L, LUA_REGISTRYINDEX, object_ref_table_name);
  lua_pushvalue (L, key_index);
  lua_rawget (L, -2);

  if (! lua_isnil (L, -1))
    {
      int ref;

      lua_pushvalue (L, val_index);
      lua_pushvalue (L, val_index);
      lua_rawget (L, -3);

      ref = lua_tointeger (L, -1);
      lua_pop (L, 1);

      if (ref <= 1)
	lua_pushnil (L);
      else
	lua_pushinteger (L, ref - 1);

      lua_rawset (L, -3);
    }

  lua_pop (L, 2);
}
