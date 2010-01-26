
/* gradcheck.c
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

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_deriv.h>
#include <gsl/gsl_rng.h>

#include "multimin.h"
#include "matrix.h"
#include "random.h"

struct grad_check {
  lua_State *L;
  int f_index, x_index;
  double step;
  size_t var_vector_index;
  size_t n;
};

static double
f_check_hook (double x, void *_p)
{
  struct grad_check *p = _p;
  lua_State *L = p->L;
  gsl_matrix *xm = matrix_check (L, p->x_index);
  gsl_matrix_set (xm, p->var_vector_index, 0, x);
  lua_pushvalue (L, p->f_index);
  lua_pushvalue (L, p->x_index);
  lua_call (p->L, 1, 1);

  if (lua_isnumber (L, -1))
    {
      double v = lua_tonumber (L, -1);
      lua_pop (L, 1);
      return v;
    }
  
  return luaL_error (L, "function should return a real number");
}

void
fdfmultimin_check_gradient (lua_State *L, struct grad_check *gc)
{
  const double abs_err = 1e-5, rel_err = 1e-5;
  gsl_matrix *x = matrix_check (L, gc->x_index);
  gsl_matrix *gnum, *gfunc;
  gsl_function F[1];
  size_t k;

  gnum = matrix_push_raw (L, gc->n, 1);

  F->function = & f_check_hook;
  F->params   = gc;

  for (k = 0; k < gc->n; k++)
    {
      double result, abserr;
      double x_start = gsl_matrix_get (x, k, 0);
      gc->var_vector_index = k;
      gsl_deriv_central (F, x_start, gc->step / 50.0, &result, &abserr);
      gsl_matrix_set (x, k, 0, x_start);
      gsl_matrix_set (gnum, k, 0, result);
    }

  gfunc = matrix_push (L, gc->n, 1);

  lua_pushvalue (L, gc->f_index);
  lua_pushvalue (L, gc->x_index);
  lua_pushvalue (L, -3); // push gfunc

  lua_call (L, 2, 0);

  for (k = 0; k < gc->n; k++)
    {
      double gn, gf;
      gn = gsl_matrix_get (gnum,  k, 0);
      gf = gsl_matrix_get (gfunc, k, 0);
      if (fabs(gn - gf) > abs_err && fabs(gn - gf) >= fabs(gn) * rel_err)
	luaL_error (L, "component #%d of gradient is wrong, "
		    "should be %f but the function gives %f", k+1, gn, gf);
    }

  lua_pop (L, 2);
}

int
gradient_auto_check (lua_State *L)
{
  gsl_matrix *x = matrix_check (L, 2);
  double step = luaL_checknumber (L, 3);
  gsl_rng *r = push_rng (L, gsl_rng_default);
  size_t k, count, nb_tests = 3, n = x->size1;
  gsl_matrix *xrnd = matrix_push_raw (L, n, 1);
  struct grad_check gc[1];

  gc->L       = L;
  gc->x_index = lua_gettop (L);
  gc->f_index = 1;
  gc->step    = step;
  gc->n       = n;

  for (count = 0; count < nb_tests; count++)
    {
      for (k = 0; k < n; k++)
	{
	  double u = gsl_matrix_get (x, k, 0) + step * gsl_rng_uniform (r);
	  gsl_matrix_set (xrnd, k, 0, u);
	}

      fdfmultimin_check_gradient (L, gc);
    }

  return 0;
}
