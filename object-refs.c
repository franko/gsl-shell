
#include <lua.h>
#include <lauxlib.h>
#include <assert.h>

#include "lua-defs.h"
#include "refs.h"
#include "object-refs.h"

static char const * const table_name[] = {"GSL.oirfs.wp", "GSL.oirfs.plt"};
static bool table_mult_valued[] = {false, true};

void
object_refs_prepare (lua_State *L)
{
  assert (table_window_plot == 0 && table_plot_obj == 1);
  register_ref_table (L, table_name[0]);
  register_ref_table (L, table_name[1]);
}

static void
mult_ref_add (lua_State *L, int index, int table_index, int value_index)
{
  int n;
  INDEX_SET_ABS(L, table_index);

  lua_rawgeti (L, table_index, index);
  if (lua_isnil (L, -1))
    {
      lua_pop (L, 1);
      lua_newtable (L);
      lua_pushvalue (L, -1);
      lua_rawseti (L, table_index, index);
      n = 0;
    }
  else
    {
      n = lua_objlen (L, -1);
    }

  lua_pushvalue (L, value_index);
  lua_rawseti (L, -2, n+1);
  lua_pop (L, 1);
}

void
object_refs_add (lua_State *L, int table, int index, 
		 int key_index, int value_index)
{
  bool mult = table_mult_valued[table];

  INDEX_SET_ABS_2(L, key_index, value_index);

  lua_getfield (L, LUA_REGISTRYINDEX, table_name[table]);
  lua_pushvalue (L, key_index);
  lua_rawget (L, -2);

  if (lua_isnil (L, -1))
    {
      lua_pop (L, 1);
      lua_newtable (L);
      lua_pushvalue (L, key_index);
      lua_pushvalue (L, -2);
      lua_rawset (L, -4);
    }

  if (mult)
    mult_ref_add (L, index, -1, value_index);
  else
    {
      lua_pushvalue (L, value_index);
      lua_rawseti (L, -2, index);
    }

  lua_pop (L, 2);
}

void
object_refs_remove (lua_State *L, int table, int index, int key_index)
{
  INDEX_SET_ABS(L, key_index);

  lua_getfield (L, LUA_REGISTRYINDEX, table_name[table]);
  lua_pushvalue (L, key_index);
  lua_rawget (L, -2);

  if (! lua_isnil (L, -1))
    {
      lua_pushnil (L);
      lua_rawseti (L, -2, index);
    }

  lua_pop (L, 2);
}

void
object_refs_lookup_apply (lua_State *L, int table, int value_index, 
			  lua_CFunction func)
{
  assert (! table_mult_valued[table]);

  INDEX_SET_ABS(L, value_index);

  lua_getfield (L, LUA_REGISTRYINDEX, table_name[table]);
  lua_pushnil (L);

  while (lua_next (L, -2) != 0)
    {
      lua_pushnil (L);

      while (lua_next (L, -2) != 0)
	{
	  if (lua_rawequal (L, -1, value_index))
	    {
	      lua_pushcfunction (L, func);
	      lua_pushvalue (L, -5);
	      lua_pushvalue (L, -4);
	      lua_call (L, 2, 0);
	    }
	  lua_pop (L, 1);
	}

      lua_pop (L, 1);
    }

  lua_pop (L, 1);
}
