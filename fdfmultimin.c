
/* fdfmultimin.c
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
#include <string.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_deriv.h>
#include <gsl/gsl_rng.h>

#include "multimin.h"
#include "random.h"
#include "lua-utils.h"
#include "matrix.h"

#include <lua.h>
#include <lauxlib.h>

static int    fdfmultimin_free     (lua_State *L);
static int    fdfmultimin_set      (lua_State *L);
static int    fdfmultimin_run      (lua_State *L);
static int    fdfmultimin_step     (lua_State *L);
static int    fdfmultimin_index    (lua_State *L);

static int    fdfmultimin_get_x        (lua_State *L);
static int    fdfmultimin_get_value    (lua_State *L);
static int    fdfmultimin_get_gradient (lua_State *L);

static double fdfmultimin_f_hook   (const gsl_vector * x, void * p);
static void   fdfmultimin_df_hook  (const gsl_vector * x, void * p, gsl_vector *G);
static void   fdfmultimin_fdf_hook (const gsl_vector * x, void * p, double *f, 
				    gsl_vector *G);

static void   fdfmultimin_check_gradient (lua_State *L, int x_index, 
					  double step_size, size_t n);

struct params {
  lua_State *L;
  size_t n;
  int type_error;
  int args_index;
};

struct fdfmultimin {
  gsl_multimin_fdfminimizer *s;
  gsl_multimin_function_fdf fdf[1];
  struct params p[1];
};

struct grad_check {
  lua_State *L;
  int f_index, x_index;
  size_t var_vector_index;
};

enum fenvidx {
  FENV_FUNCTION = 1,
  FENV_X        = 2,
  FENV_GRAD     = 3
};

char const * const fdfmultimin_mt_name = "GSL.mmin";

static const struct luaL_Reg fdfmultimin_properties[] = {
  {"x",            fdfmultimin_get_x},
  {"value",        fdfmultimin_get_value},
  {"gradient",     fdfmultimin_get_gradient},
  {NULL, NULL}
};

const struct luaL_Reg fdfmultimin_methods[] = {
  {"__gc",         fdfmultimin_free},
  {"__index",      fdfmultimin_index},
  {"step",         fdfmultimin_step},
  {"run",          fdfmultimin_run},
  {"set",          fdfmultimin_set},
  {NULL, NULL}
};

#define MULTIMIN_MAX_ITER 20

struct fdfmultimin *
push_new_fdf_multimin (lua_State *L, int findex, size_t n,
		       const gsl_multimin_fdfminimizer_type *T)
{
  struct fdfmultimin *m = lua_newuserdata (L, sizeof (struct fdfmultimin));
  m->s = gsl_multimin_fdfminimizer_alloc (T, n);

  if (m->s == NULL)
    luaL_error (L, OUT_OF_MEMORY_MSG);

  luaL_getmetatable (L, fdfmultimin_mt_name);
  lua_setmetatable (L, -2);

  lua_newtable (L);

  lua_pushvalue (L, findex);
  lua_rawseti (L, -2, FENV_FUNCTION);

  matrix_push_raw (L, n, 1);
  lua_rawseti (L, -2, FENV_X);

  matrix_push_view (L, NULL);
  lua_rawseti (L, -2, FENV_GRAD);

  lua_setfenv (L, -2);

  return m;
}

static struct fdfmultimin *
check_fdf_multimin (lua_State *L, int index)
{
  return luaL_checkudata (L, index, fdfmultimin_mt_name);
}

static struct fdfmultimin *
check_init_fdf_multimin (lua_State *L, int index)
{
  struct fdfmultimin *m = luaL_checkudata (L, index, fdfmultimin_mt_name);
  if (m->fdf->params == NULL)
    luaL_error (L, "minimizer is not initialised");
  return m;
}

int
fdfmultimin_new (lua_State *L)
{
  struct fdfmultimin *m;
  int ni = luaL_checkinteger (L, 2);
  int findex = 1;
  size_t n;

  if (ni <= 0)
    luaL_error (L, "argument #1 should be a positive integer");
  else
    n = (size_t) ni;

  if (!lua_isfunction (L, findex))
    luaL_typerror (L, findex, "function");

  m = push_new_fdf_multimin (L, findex, n, gsl_multimin_fdfminimizer_conjugate_fr);

  m->fdf->n      = n;
  m->fdf->f      = fdfmultimin_f_hook;
  m->fdf->df     = fdfmultimin_df_hook;
  m->fdf->fdf    = fdfmultimin_fdf_hook;
  m->fdf->params = NULL;

  return 1;
}

int
fdfmultimin_free (lua_State *L)
{
  struct fdfmultimin *m = check_fdf_multimin (L, 1);
  lua_getfenv (L, 1);
  lua_rawgeti (L, 2, FENV_GRAD);
  matrix_null_view (L, -1);
  gsl_multimin_fdfminimizer_free (m->s);
  return 0;
}

void
fdfmultimin_fdf_hook (const gsl_vector * x, void * _p, double *f, gsl_vector *G)
{
  struct params *p = _p;
  lua_State *L = p->L;
  size_t n = p->n, k;
  gsl_matrix *lx;
  int aindex = p->args_index;
  int nargs = 1;

  lua_pushvalue (L, aindex);

  lx = matrix_check (L, aindex+1);
  for (k = 0; k < n; k++)
    lx->data[k] = x->data[k];
  lua_pushvalue (L, aindex+1);

  if (G)
    {
      matrix_set_view_and_push (L, aindex+2, G->data, n, 1, NULL);
      nargs ++;
    }

  lua_call (L, nargs, 1);

  if (lua_isnumber (L, -1))
    *f = lua_tonumber (L, -1);
  else
    {
      *f = GSL_NAN;
      p->type_error = 1;
    }

  lua_pop (L, 1);
}

double
fdfmultimin_f_hook (const gsl_vector * x, void * params)
{
  double f;
  fdfmultimin_fdf_hook (x, params, &f, NULL);
  return f;
}

void
fdfmultimin_df_hook (const gsl_vector * x, void * params, gsl_vector *G)
{
  double f;
  fdfmultimin_fdf_hook (x, params, &f, G);
}

static void
fcall_args_prepare (lua_State *L, struct params *p, int index)
{
  int bindex;
  lua_getfenv (L, index);

  bindex = lua_gettop  (L);
  lua_rawgeti (L, bindex, FENV_FUNCTION);
  lua_rawgeti (L, bindex, FENV_X);
  lua_rawgeti (L, bindex, FENV_GRAD);
  lua_remove  (L, bindex);

  p->args_index = bindex;
} 

static void
fcall_args_remove (lua_State *L)
{
  lua_pop (L, 3);
} 

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
fdfmultimin_check_gradient (lua_State *L, int x_index, double step_size, size_t n)
{
  const double abs_err = 1e-5, rel_err = 1e-5;
  gsl_matrix *x = matrix_check (L, x_index);
  struct grad_check gc[1];
  gsl_matrix *gnum, *gfunc;
  gsl_function F[1];
  size_t k;

  gnum = matrix_push_raw (L, n, 1);

  lua_getfenv (L, 1);
  lua_rawgeti (L, -1, FENV_FUNCTION);

  gc->L = L;
  gc->x_index = x_index;
  gc->f_index = lua_gettop (L);

  F->function = & f_check_hook;
  F->params   = gc;

  for (k = 0; k < n; k++)
    {
      double result, abserr;
      double x_start = gsl_matrix_get (x, k, 0);
      gc->var_vector_index = k;
      gsl_deriv_central (F, x_start, step_size / 50.0, &result, &abserr);
      gsl_matrix_set (x, k, 0, x_start);
      gsl_matrix_set (gnum, k, 0, result);
    }

  gfunc = matrix_push (L, n, 1);

  lua_pushvalue (L, gc->f_index);
  lua_pushvalue (L, gc->x_index);
  lua_pushvalue (L, -3); // push gfunc

  lua_call (L, 2, 0);

  for (k = 0; k < n; k++)
    {
      double gn, gf;
      gn = gsl_matrix_get (gnum,  k, 0);
      gf = gsl_matrix_get (gfunc, k, 0);
      if (fabs(gn - gf) > abs_err && fabs(gn - gf) >= fabs(gn) * rel_err)
	luaL_error (L, "component #%d of gradient is wrong, "
		    "should be %f but the function gives %f", k+1, gn, gf);
    }

  lua_pop (L, 4);
}

static void
gradient_auto_check (lua_State *L, gsl_matrix *x, double step, size_t n)
{
  gsl_rng *r = push_rng (L, gsl_rng_default);
  gsl_matrix *xrnd = matrix_push_raw (L, n, 1);
  size_t k, count, nb_tests = 3;
  int xrnd_index = lua_gettop (L);

  for (count = 0; count < nb_tests; count++)
    {
      for (k = 0; k < n; k++)
	{
	  double u = gsl_matrix_get (x, k, 0) + step * gsl_rng_uniform (r);
	  gsl_matrix_set (xrnd, k, 0, u);
	}

      fdfmultimin_check_gradient (L, xrnd_index, step, n);
    }

  lua_pop (L, 2);
}

int
fdfmultimin_set (lua_State *L)
{
  struct fdfmultimin *m = check_fdf_multimin (L, 1);
  gsl_matrix *x0m = matrix_check (L, 2);
  double step_size;
  gsl_vector_view x0v;
  size_t n = m->fdf->n;
  int status;

  if (lua_isnumber (L, 3))
    {
      step_size = lua_tonumber (L, 3);
    }
  else 
    {
      gsl_matrix *sm = matrix_check (L, 3);
      gsl_vector_view sv = gsl_matrix_column (sm, 0);
      step_size = geometric_mean (L, &sv.vector);
    }

  lua_settop (L, 2);  /* get rid of the step_size */

  m->fdf->params = m->p;

  m->p->L = L;
  m->p->n = n;
  m->p->type_error = 0;

  x0v = gsl_matrix_column (x0m, 0);

  mlua_null_cache (L, 1);
  fcall_args_prepare (L, m->p, 1);

  status = gsl_multimin_fdfminimizer_set (m->s, m->fdf, &x0v.vector, step_size, 0.1);

  if (status != GSL_SUCCESS)
    return luaL_error (L, "minimizer:set %s", gsl_strerror (status));

  fcall_args_remove (L);
  gradient_auto_check (L, x0m, step_size, n);

  return 0;
}
    
int
fdfmultimin_step (lua_State *L)
{
  struct fdfmultimin *m = check_fdf_multimin (L, 1);
  int status;

  mlua_null_cache (L, 1);
  fcall_args_prepare (L, m->p, 1);

  status = gsl_multimin_fdfminimizer_iterate (m->s);

  if (status != GSL_SUCCESS)
    return luaL_error (L, "minimizer:step %s", gsl_strerror (status));

  status = gsl_multimin_test_gradient (m->s->gradient, 1e-4);

  if (status == GSL_CONTINUE)
    {
      lua_pushstring (L, "continue");
      return 1;
    }
  else if (status == GSL_SUCCESS)
    {
      lua_pushstring (L, "success");
      return 1;
    }

  return luaL_error (L, "minimizer:step %s", gsl_strerror (status));
}

int
fdfmultimin_run (lua_State *L)
{
  struct fdfmultimin *m = check_fdf_multimin (L, 1);
  size_t iter = 0;
  int status;

  mlua_null_cache (L, 1);
  fcall_args_prepare (L, m->p, 1);

  do {
    iter++;

    status = gsl_multimin_fdfminimizer_iterate (m->s);

    if (status)
      break;

    status = gsl_multimin_test_gradient (m->s->gradient, 1e-4);

    if (status == GSL_SUCCESS)
      return 0;
  }
  while (status == GSL_CONTINUE && iter < MULTIMIN_MAX_ITER);

  if (m->p->type_error)
    return luaL_error (L, "function should return a real number");

  return luaL_error (L, "minimizer:run %s", gsl_strerror (status));
}

int
fdfmultimin_get_gradient (lua_State *L)
{
  struct fdfmultimin *m = check_init_fdf_multimin (L, 1);
  gsl_matrix *gm = matrix_push (L, m->p->n, 1);
  gsl_vector *g = gsl_multimin_fdfminimizer_gradient (m->s);
  size_t k;
  for (k = 0; k < m->p->n; k++)
    gm->data[k] = g->data[k];
  return 1;
}

int
fdfmultimin_get_x (lua_State *L)
{
  struct fdfmultimin *m = check_init_fdf_multimin (L, 1);
  gsl_matrix *xm = matrix_push (L, m->p->n, 1);
  gsl_vector *x = gsl_multimin_fdfminimizer_x (m->s);
  size_t k;
  for (k = 0; k < m->p->n; k++)
    xm->data[k] = x->data[k];
  return 1;
}

int
fdfmultimin_get_value (lua_State *L)
{
  struct fdfmultimin *m = check_init_fdf_multimin (L, 1);
  double v = gsl_multimin_fdfminimizer_minimum (m->s);
  lua_pushnumber (L, v);
  return 1;
}

int
fdfmultimin_index (lua_State *L)
{
  return mlua_index_with_properties (L, fdfmultimin_properties, true);
}
