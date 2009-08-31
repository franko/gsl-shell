
/* fdfsolver.c
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
#include <assert.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit_nlin.h>

#include "fdfsolver.h"

#define NLINFIT_MAX_ITER 30

char const * const fdfsolver_mt_name = "GSL.fdfsolver";

const struct luaL_Reg fdfsolver_methods[] = {
  {"__gc",          fdfsolver_dealloc},
  {NULL, NULL}
};

int
fdfsolver_dealloc (lua_State *L)
{
  struct fdfsolver *fdf = check_fdfsolver (L, 1);
  gsl_multifit_fdfsolver_free (fdf->base);
  gsl_vector_free (fdf->fit_data->x);
  if (fdf->fit_data->j_raw)
    gsl_vector_free (fdf->fit_data->j_raw);
  return 0;
}

struct fdfsolver *
check_fdfsolver (lua_State *L, int index)
{
  return luaL_checkudata (L, index, fdfsolver_mt_name);
}

struct fdfsolver *
push_new_fdfsolver (lua_State *L, size_t n, size_t p)
{
  gsl_multifit_fdfsolver_type const * const T = gsl_multifit_fdfsolver_lmsder;
  struct fdfsolver *sext = lua_newuserdata (L, sizeof (struct fdfsolver));
 
  sext->base = gsl_multifit_fdfsolver_alloc (T, n, p);

  if (sext->base == NULL)
    luaL_error (L, OUT_OF_MEMORY_MSG);

  luaL_getmetatable (L, fdfsolver_mt_name);
  lua_setmetatable (L, -2);

  return sext;
}
