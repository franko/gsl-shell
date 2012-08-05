
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
#include "window_hooks.h"

#ifndef MLUA_GRAPHLIBNAME
#define MLUA_GRAPHLIBNAME "graph"
#endif

static const struct luaL_Reg methods_dummy[] = {{NULL, NULL}};

pthread_mutex_t agg_mutex[1];

void
graph_close_windows (lua_State *L)
{
  window_index_apply_all (L, window_close_wait);
}

void
register_graph (lua_State *L)
{
  pthread_mutex_init (agg_mutex, NULL);

  window_registry_prepare (L);

  luaL_register (L, MLUA_GRAPHLIBNAME, methods_dummy);

  draw_register (L);
  text_register (L);
  app_window_hooks->register_module (L);
  plot_register (L);

  initialize_fonts (L);

  lua_pop(L, 1);
}

void
gsl_shell_close_with_graph (struct gsl_shell_state* gs)
{
  pthread_mutex_lock (&gs->shutdown_mutex);
  gs->is_shutting_down = 1;
  pthread_mutex_lock(&gs->exec_mutex);
  graph_close_windows(gs->L);
  lua_close(gs->L);
  pthread_mutex_unlock(&gs->shutdown_mutex);
  pthread_mutex_unlock(&gs->exec_mutex);
}
