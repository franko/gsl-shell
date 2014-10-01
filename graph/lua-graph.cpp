
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
#include "fonts.h"
#include "window_registry.h"
#include "lua-draw.h"
#include "lua-text.h"
#include "window.h"
#include "lua-plot.h"
#include "window_hooks.h"

int
stub_window_fn(lua_State *L)
{
    return luaL_error(L, "no windows module loaded");
}

struct window_hooks stub_window_hooks[1] = {{
    stub_window_fn, stub_window_fn, stub_window_fn, stub_window_fn,
    stub_window_fn, stub_window_fn, stub_window_fn, stub_window_fn,
    stub_window_fn,
}};

pthread_mutex_t agg_mutex[1];

struct window_hooks *app_window_hooks = stub_window_hooks;

static void register_window_hooks(struct window_hooks *w)
{
    app_window_hooks = w;
}

struct graphics_hooks graphics[1] = {
    register_window_hooks, agg_mutex,
};

int
luaopen_graphcore(lua_State *L)
{
    pthread_mutex_init(agg_mutex, NULL);
    window_registry_prepare(L);
    lua_newtable(L);
    draw_register(L);
    text_register(L);
    plot_register(L);
    initialize_fonts(L);

    lua_pushlightuserdata(L, graphics);
    lua_setfield(L, LUA_REGISTRYINDEX, "github.com/franko/libgraph");

    return 1;
}
