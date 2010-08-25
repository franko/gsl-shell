
#include <lua.h>
#include <lauxlib.h>

#include "lua-defs.h"
#include "refs.h"
#include "win-plot-refs.h"

static char const * const window_plot_ref_table_name = "GSL.winpltrefs";

void
window_plot_ref_prepare (lua_State *L)
{
  register_ref_table (L, window_plot_ref_table_name);
}

void
window_plot_ref_add (lua_State *L, int slot_id, int window_index, int plot_index)
{
  INDEX_SET_ABS_2(L, window_index, plot_index);

  lua_getfield (L, LUA_REGISTRYINDEX, window_plot_ref_table_name);
  lua_pushvalue (L, window_index);
  lua_rawget (L, -2);

  if (lua_isnil (L, -1))
    {
      lua_pop (L, 1);
      lua_newtable (L);
      lua_pushvalue (L, plot_index);
      lua_rawseti (L, -2, slot_id);

      lua_pushvalue (L, window_index);
      lua_insert (L, -2);
      lua_rawset (L, -3);
    }
  else
    {
      lua_pushvalue (L, plot_index);
      lua_rawseti (L, -2, slot_id);
    }

  lua_pop (L, 2);
}

void
window_plot_ref_remove (lua_State *L, int slot_id, int window_index)
{
  INDEX_SET_ABS(L, window_index);

  lua_getfield (L, LUA_REGISTRYINDEX, window_plot_ref_table_name);
  lua_pushvalue (L, window_index);
  lua_rawget (L, -2);

  if (! lua_isnil (L, -1))
    {
      lua_pushnil (L);
      lua_rawseti (L, -2, slot_id);
    }

  lua_pop (L, 2);
}
