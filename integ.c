
/* integ.c
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
#include <gsl/gsl_integration.h>

#include "integ.h"

static int integ (lua_State *L);

static const struct luaL_Reg integ_functions[] = {
  {"integ",     integ},
  {NULL, NULL}
};

static int
get_parameter (lua_State *L, const char *name, double *val, bool required)
{
  lua_getfield (L, 1, name);
  if (lua_isnil (L, -1))
    {
      if (required)
	luaL_error (L, "parameter %s not provided", name);
      lua_pop (L, 1);
      return 1;
    }
  *val = lua_tonumber (L, -1);
  lua_pop (L, 1);
  return 0;
}

static double
integ_hook_f (double x, void *params)
{
  lua_State *L = params;
  double y;
  lua_pushvalue (L, 2);
  lua_pushnumber (L, x);
  lua_call (L, 1, 1);
  y = lua_tonumber (L, -1);
  lua_pop (L, 1);
  return y;
}

int
integ (lua_State *L)
{
  const size_t limit = 1000;
  gsl_integration_workspace *w;
  gsl_function f[1];
  double a, b;
  double result, error;
  const double epsabs = 1e-7, epsrel = 1e-7;
  int status;

  luaL_checktype (L, 1, LUA_TTABLE);

  lua_getfield (L, 1, "f");

  get_parameter (L, "a", &a, true);
  get_parameter (L, "b", &b, true);

  f->function = & integ_hook_f;
  f->params   = L;
  
  w = gsl_integration_workspace_alloc (limit);

  status = gsl_integration_qags (f, a, b, epsabs, epsrel, limit,
				 w, &result, &error);

  gsl_integration_workspace_free (w);

  lua_pop (L, 1);

  lua_pushnumber (L, result);
  lua_pushnumber (L, error);

  return 2;
}

void
integ_register (lua_State *L)
{
  /* gsl module registration */
  luaL_register (L, NULL, integ_functions);
}
