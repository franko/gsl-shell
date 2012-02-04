
/* lua-gsl.c
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

#include <lua.h>
#include <lauxlib.h>

#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>

#include "lua-gsl.h"
#include "gs-types.h"
#include "lua-utils.h"

#include "lua-graph.h"

int
luaopen_gsl (lua_State *L)
{
  gsl_set_error_handler_off ();

  luaL_register (L, "gslsh", gs_type_functions);
  lua_pop (L, 1);

  register_graph (L);

  return 1;
}
