
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
#include "object-index.h"
#include "window.h"
#include "gs-types.h"
#include "lua-utils.h"
#include "object-refs.h"
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
static int plot_add_line   (lua_State *L);
static int plot_index      (lua_State *L);
static int plot_newindex   (lua_State *L);
static int plot_free       (lua_State *L);
// static int plot_show       (lua_State *L);
static int plot_title_set  (lua_State *L);
static int plot_title_get  (lua_State *L);
static int plot_units_set  (lua_State *L);
static int plot_units_get  (lua_State *L);

static int plot_add_gener (lua_State *L, bool as_line);

static const struct luaL_Reg plot_functions[] = {
  {"plot",        plot_new},
  {NULL, NULL}
};

static const struct luaL_Reg plot_methods[] = {
  {"add",         plot_add        },
  {"addline",     plot_add_line   },
  {"update",      plot_update     },
  //  {"show",        plot_show       },
  {"__index",     plot_index      },
  {"__newindex",  plot_newindex   },
  {"__gc",        plot_free       },
  {NULL, NULL}
};

static const struct luaL_Reg plot_properties_get[] = {
  {"title",        plot_title_get  },
  {"units",        plot_units_get  },
  {NULL, NULL}
};

static const struct luaL_Reg plot_properties_set[] = {
  {"title",        plot_title_set  },
  {"units",        plot_units_set  },
  {NULL, NULL}
};

__END_DECLS

lua_plot *
lua_plot::check(lua_State *L, int index)
{
  return (lua_plot *) gs_check_userdata (L, index, GS_PLOT);
}

void
lua_plot::update_window(lua_State *L)
{
  if (this->window_id <= 0)
    return;

  object_index_get (L, OBJECT_WINDOW, this->window_id);

  if (gs_is_userdata (L, lua_gettop (L), GS_WINDOW))
    {
      lua_pushcfunction (L, window_slot_update_unprotected);
      lua_insert (L, -2);
      lua_pushinteger (L, this->slot_id);
      lua_call (L, 2, 0);
    }
  else
    {
      lua_pop (L, 1);
    }
}

int
plot_new (lua_State *L)
{
  lua_plot *p = new(L, GS_PLOT) lua_plot();

  if (lua_isstring (L, 1))
    {
      const char *title = lua_tostring (L, 1);
      if (title)
	p->self().set_title(title);
    }

  p->id = object_index_add (L, OBJECT_PLOT, -1);

  return 1;
}

int
plot_free (lua_State *L)
{
  lua_plot *p = lua_plot::check(L, 1);
  p->~lua_plot();
  return 0;
}

int
plot_add_gener (lua_State *L, bool as_line)
{
  lua_plot *p = lua_plot::check(L, 1);
  drawable *obj = parse_graph_args (L);
  agg::rgba8 *color = check_color_rgba8 (L, 3);

  object_ref_add (L, 1, 2);

  AGG_LOCK();

  p->self().add(obj, color, as_line);
  p->update_window(L);

  AGG_UNLOCK();

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
  lua_plot *p = lua_plot::check(L, 1);
  const char *title = lua_tostring (L, 2);

  if (title == NULL)
    return gs_type_error (L, 2, "string");
	  
  AGG_LOCK();

  p->self().set_title(title);
  p->update_window(L);

  AGG_UNLOCK();
	  
  return 0;
}

int
plot_title_get (lua_State *L)
{
  lua_plot *p = lua_plot::check(L, 1);

  AGG_LOCK();

  const char *title = p->self().get_title();
  lua_pushstring (L, title);

  AGG_UNLOCK();
  
  return 1;
}

int
plot_units_set (lua_State *L)
{
  lua_plot *p = lua_plot::check(L, 1);
  bool request = (bool) lua_toboolean (L, 2);

  AGG_LOCK();
  
  lua_plot::plot_type& plt = p->self();
  bool current = plt.use_units();

  if (current != request)
    {
      plt.set_units(request);
      p->update_window(L);
    }

  AGG_UNLOCK();
	  
  return 0;
}

int
plot_units_get (lua_State *L)
{
  lua_plot *p = lua_plot::check(L, 1);

  AGG_LOCK();

  lua_plot::plot_type& plt = p->self();
  lua_pushboolean (L, plt.use_units());

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

int
plot_update (lua_State *L)
{
  lua_plot *p = lua_plot::check(L, 1);
  AGG_LOCK();
  p->update_window(L);
  AGG_UNLOCK();
  return 0;
}

/*
int
plot_show (lua_State *L)
{
  lua_plot *p = lua_plot::check(L, 1);
  p->start_new_thread (L);
  return 1;
}
*/

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
