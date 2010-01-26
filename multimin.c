
/* multimin.c
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

#include <math.h>

#include "multimin.h"

static int multimin_new (lua_State *L);

static const struct luaL_Reg multimin_functions[] = {
  {"minimizer",      multimin_new},
  {"gradcheck",      gradient_auto_check},
  {NULL, NULL}
};

double
geometric_mean (lua_State *L, gsl_vector *v)
{
  size_t k, n = v->size;
  double p = 0.0;

  for (k = 0; k < n; k++)
    {
      double x = gsl_vector_get (v, k);
      if (x <= 0.0)
	luaL_error (L, "invalid step: negative element at index %d", k+1);
      p += log(x);
    }

  return exp(p / n);
}

int
multimin_new (lua_State *L)
{
  size_t n;
  int use_fdf = 0;

  if (lua_type (L, 1) != LUA_TTABLE)
    return luaL_error (L, "minimizer should be created with a table "
		       "of parameters");

  lua_settop (L, 1);

  lua_getfield (L, 1, "f");
  if (lua_isnil (L, -1))
    {
      lua_pop (L, 1);
      lua_getfield (L, 1, "fdf");
      if (lua_isnil (L, -1))
	return luaL_error (L, "f or fdf expected");
      use_fdf = 1;
    }

  lua_getfield (L, 1, "n");
  n = lua_tointeger (L, -1);
  if (n <= 0)
    return luaL_error (L, "\"n\" should be given as a positive integer");
  lua_remove (L, 1);
  return (use_fdf ? fdfmultimin_new (L) : fmultimin_new (L));
}

void
multimin_register (lua_State *L)
{
  luaL_newmetatable (L, fmultimin_mt_name);
  luaL_register (L, NULL, fmultimin_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, fdfmultimin_mt_name);
  luaL_register (L, NULL, fdfmultimin_methods);
  lua_pop (L, 1);

  luaL_register (L, NULL, multimin_functions);
}
