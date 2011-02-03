
/* lua-plot.cpp
 * 
 * Copyright (C) 2009, 2010 Francesco Abbate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "lua-plot.h"
#include "lua-plot-cpp.h"
#include "lua-cpp-utils.h"
#include "bitmap-plot.h"
#include "window.h"
#include "gs-types.h"
#include "lua-utils.h"
#include "window_registry.h"
#include "lua-cpp-utils.h"
#include "lua-draw.h"
#include "colors.h"
#include "plot.h"
#include "drawable.h"
#include "resource-manager.h"
#include "agg-parse-trans.h"

__BEGIN_DECLS

static int plot_new        (lua_State *L);
static int plot_add        (lua_State *L);
static int plot_update     (lua_State *L);
static int plot_flush      (lua_State *L);
static int plot_add_line   (lua_State *L);
static int plot_index      (lua_State *L);
static int plot_newindex   (lua_State *L);
static int plot_free       (lua_State *L);
static int plot_show       (lua_State *L);
static int plot_title_set  (lua_State *L);
static int plot_title_get  (lua_State *L);
static int plot_units_set  (lua_State *L);
static int plot_units_get  (lua_State *L);
static int plot_set_limits (lua_State *L);
static int plot_push_layer (lua_State *L);
static int plot_pop_layer  (lua_State *L);
static int plot_clear      (lua_State *L);

static int plot_sync_mode_get (lua_State *L);
static int plot_sync_mode_set (lua_State *L);
static int plot_pad_mode_get (lua_State *L);
static int plot_pad_mode_set (lua_State *L);

static int canvas_new      (lua_State *L);

static int   plot_add_gener  (lua_State *L, bool as_line);
static void  plot_update_raw (lua_State *L, lua_plot *p, int plot_index);

static const struct luaL_Reg plot_functions[] = {
  {"plot",        plot_new},
  {"canvas",      canvas_new},
  {NULL, NULL}
};

static const struct luaL_Reg plot_methods[] = {
  {"add",         plot_add        },
  {"addline",     plot_add_line   },
  {"update",      plot_update     },
  {"flush",       plot_flush      },
  {"show",        plot_show       },
  {"limits",      plot_set_limits },
  {"pushlayer",   plot_push_layer },
  {"poplayer",    plot_pop_layer  },
  {"clear",       plot_clear      },
  {"save",        bitmap_save_image },
  {"__index",     plot_index      },
  {"__newindex",  plot_newindex   },
  {"__gc",        plot_free       },
  {NULL, NULL}
};

static const struct luaL_Reg plot_properties_get[] = {
  {"title",        plot_title_get  },
  {"units",        plot_units_get  },
  {"sync",         plot_sync_mode_get  },
  {"pad",          plot_pad_mode_get  },
  {NULL, NULL}
};

static const struct luaL_Reg plot_properties_set[] = {
  {"title",        plot_title_set  },
  {"units",        plot_units_set  },
  {"sync",         plot_sync_mode_set  },
  {"pad",          plot_pad_mode_set  },
  {NULL, NULL}
};

__END_DECLS

int
plot_new (lua_State *L)
{
  typedef plot_auto<drawable, lua_management> plot_type;
  lua_plot *p = push_new_object<plot_type>(L, GS_PLOT);

  lua_newtable (L);
  lua_setfenv (L, -2);

  if (lua_isstring (L, 1))
    {
      const char *title = lua_tostring (L, 1);
      if (title)
	p->set_title(title);
    }

  return 1;
}

int
canvas_new (lua_State *L)
{
  lua_plot *p = push_new_object<lua_plot>(L, GS_PLOT);

  lua_newtable (L);
  lua_setfenv (L, -2);

  p->sync_mode(false);

  if (lua_isstring (L, 1))
    {
      const char *title = lua_tostring (L, 1);
      if (title)
	p->set_title(title);
    }

  return 1;
}

int
plot_free (lua_State *L)
{
  return object_free<lua_plot>(L, 1, GS_PLOT);
}

void
plot_add_gener_cpp (lua_State *L, lua_plot *p, bool as_line, 
		    gslshell::ret_status& st)
{
  try {
    agg::rgba8 color;
    drawable *obj = parse_graph_args (L, color);

    AGG_LOCK();
    p->add(obj, color, as_line);
    AGG_UNLOCK();

    if (p->sync_mode())
      plot_flush (L);
  }
  catch (std::exception& e)
    {
      st.error(e.what(), "plot add or addline");
    }
}

static void
objref_mref_add (lua_State *L, int table_index, int index, int value_index)
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

int
plot_add_gener (lua_State *L, bool as_line)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);

  gslshell::ret_status st;
  plot_add_gener_cpp (L, p, as_line, st);

  if (st.error_msg())
    return luaL_error (L, "%s in %s", st.error_msg(), st.context());

  lua_getfenv (L, 1);
  objref_mref_add (L, -1, p->current_layer_index(), 2);

  return 0;
}
 
int
plot_add (lua_State *L)
{
  return plot_add_gener (L, false);
}
 
int
plot_add_line (lua_State *L)
{
  return plot_add_gener (L, true);
}

int
plot_title_set (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);
  const char *title = lua_tostring (L, 2);

  if (title == NULL)
    return gs_type_error (L, 2, "string");
	  
  AGG_LOCK();
  p->set_title(title);
  AGG_UNLOCK();

  plot_update_raw (L, p, 1);
	  
  return 0;
}

int
plot_title_get (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);

  AGG_LOCK();

  const char *title = p->title();
  lua_pushstring (L, title);

  AGG_UNLOCK();
  
  return 1;
}

int
plot_units_set (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);
  bool request = (bool) lua_toboolean (L, 2);
  AGG_LOCK();
  p->set_units(request);
  AGG_UNLOCK();
  plot_update_raw (L, p, 1);
  return 0;
}

int
plot_units_get (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);

  AGG_LOCK();
  lua_pushboolean (L, p->use_units());
  AGG_UNLOCK();

  return 1;
}

int
plot_index (lua_State *L)
{
  return mlua_index_with_properties (L, plot_properties_get, false);
}

int
plot_newindex (lua_State *L)
{
  return mlua_newindex_with_properties (L, plot_properties_set);
}

void
plot_update_raw (lua_State *L, lua_plot *p, int plot_index)
{
  window_refs_lookup_apply (L, plot_index, window_slot_update);
  p->commit_pending_draw();
}

int
plot_update (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);
  plot_update_raw (L, p, 1);
  return 0;
}

int
plot_flush (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);
  window_refs_lookup_apply (L, 1, window_slot_refresh);
  p->commit_pending_draw();
  return 0;
}

int
plot_show (lua_State *L)
{
  lua_pushcfunction (L, window_attach);
  window_new (L);
  lua_pushvalue (L, 1);
  lua_pushstring (L, "");
  lua_call (L, 3, 0);
  return 0;
}

int
plot_set_limits (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);

  agg::rect_base<double> r;
  r.x1 = gs_check_number (L, 2, true);
  r.y1 = gs_check_number (L, 3, true);
  r.x2 = gs_check_number (L, 4, true);
  r.y2 = gs_check_number (L, 5, true);

  AGG_LOCK();
  p->set_limits(r);
  AGG_UNLOCK();
  plot_update_raw (L, p, 1);
  return 0;
}

int
plot_push_layer (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);

  window_refs_lookup_apply (L, 1, window_slot_refresh);

  AGG_LOCK();
  p->push_layer();
  AGG_UNLOCK();

  window_refs_lookup_apply (L, 1, window_save_slot_image);

  return 0;
}

static void
plot_ref_clear (lua_State *L, int index, int layer_id)
{
  lua_getfenv (L, index);
  lua_newtable (L);
  lua_rawseti (L, -2, layer_id);
  lua_pop (L, 1);
 }

int
plot_pop_layer (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);

  plot_ref_clear (L, 1, p->current_layer_index());

  AGG_LOCK();
  p->pop_layer();
  AGG_UNLOCK();

  plot_update_raw (L, p, 1);
  return 0;
}

int
plot_clear (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);

  plot_ref_clear (L, 1, p->current_layer_index());

  AGG_LOCK();
  p->clear_current_layer();
  AGG_UNLOCK();

  window_refs_lookup_apply (L, 1, window_restore_slot_image);

  if (p->sync_mode())
    plot_update_raw (L, p, 1);

  return 0;
}

static int plot_pad_mode_set (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);
  bool request = (bool) lua_toboolean (L, 2);
  AGG_LOCK();
  p->pad_mode(request);
  AGG_UNLOCK();
  plot_update_raw (L, p, 1);
  return 0;
}

static int plot_pad_mode_get (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);
  AGG_LOCK();
  lua_pushboolean (L, p->pad_mode());
  AGG_UNLOCK();
  return 1;
}

int
plot_sync_mode_get (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);
  AGG_LOCK();
  lua_pushboolean (L, p->sync_mode());
  AGG_UNLOCK();
  return 1;
}

int
plot_sync_mode_set (lua_State *L)
{
  lua_plot *p = object_check<lua_plot>(L, 1, GS_PLOT);
  bool request = (bool) lua_toboolean (L, 2);
  AGG_LOCK();
  p->sync_mode(request);
  AGG_UNLOCK();
  return 0;
}

void
plot_register (lua_State *L)
{
  /* plot declaration */
  luaL_newmetatable (L, GS_METATABLE(GS_PLOT));
  luaL_register (L, NULL, plot_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, plot_functions);
}
