
/* lua-graph.c
 *
 * Copyright (C) 2009 Francesco Abbate
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
#include <lua.h>
#include <lauxlib.h>
}

#include "lua-graph.h"
#include "lua-gsl.h"
#include "window_registry.h"
#include "lua-draw.h"
#include "lua-text.h"
#include "window.h"
#include "lua-plot.h"

#ifndef MLUA_GRAPHLIBNAME
#define MLUA_GRAPHLIBNAME "graph"
#endif

static const struct luaL_Reg methods_dummy[] = {{NULL, NULL}};

void
graph_close_windows (lua_State *L)
{
  window_index_apply_all (L, window_close_wait);
}

void
register_graph (lua_State *L)
{
  window_registry_prepare (L);

  luaL_register (L, MLUA_GRAPHLIBNAME, methods_dummy);

  draw_register (L);
  text_register (L);
  window_register (L);
  plot_register (L);

  lua_pop(L, 1);
}

void
lua_close_with_graph (lua_State* L)
{
  pthread_mutex_lock (gsl_shell_shutdown_mutex);
  gsl_shell_shutting_down = 1;
  GSL_SHELL_LOCK();
  graph_close_windows(L);
  lua_close(L);
  pthread_mutex_unlock (gsl_shell_shutdown_mutex);
  GSL_SHELL_UNLOCK();
}
