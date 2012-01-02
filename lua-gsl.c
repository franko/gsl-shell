
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
#include "sf.h"

#include "lua-graph.h"

#ifdef GSL_SHELL_DEBUG
static int gsl_shell_lua_registry (lua_State *L);
#endif

#ifdef GSL_SHELL_DEBUG
static const struct luaL_Reg gsl_shell_functions[] = {
  {"registry", gsl_shell_lua_registry},
  {NULL, NULL}
};
#endif

int
luaopen_gsl (lua_State *L)
{
  gsl_set_error_handler_off ();

  luaL_register (L, "gslsh", gs_type_functions);
#ifdef GSL_SHELL_DEBUG
  luaL_register (L, NULL, gsl_shell_functions);
#endif
  lua_pop (L, 1);

  register_graph (L);

  return 1;
}

#ifdef GSL_SHELL_DEBUG
int
gsl_shell_lua_registry (lua_State *L)
{
  lua_pushvalue (L, LUA_REGISTRYINDEX);
  return 1;
}
#endif
