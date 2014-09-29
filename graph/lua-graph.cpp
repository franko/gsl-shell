
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
#include "window_registry.h"
#include "lua-draw.h"
#include "lua-text.h"
#include "window.h"
#include "lua-plot.h"
#include "window_hooks.h"

struct window_hooks nat_window_hooks[1] = {{
        window_new, window_show, window_attach,
        window_slot_update, window_slot_refresh,
        window_close_wait, window_wait,
        window_save_slot_image, window_restore_slot_image,
        window_register, agg_mutex,
    }
};

pthread_mutex_t agg_mutex[1];

int
luaopen_graphcore (lua_State *L)
{
    if (!app_window_hooks) {
        app_window_hooks = nat_window_hooks;
    }

    pthread_mutex_init(agg_mutex, NULL);
    window_registry_prepare(L);
    lua_newtable(L);
    draw_register(L);
    text_register(L);
    app_window_hooks->register_module(L, app_window_hooks->graphics_mutex);
    plot_register(L);
    initialize_fonts(L);
    return 1;
}
