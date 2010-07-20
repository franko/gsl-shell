
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

#include "gs-types.h"
#include "multimin.h"
#include "random.h"
#include "lua-utils.h"
#include "matrix.h"

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

enum fenvidx {
  FENV_FUNCTION = 1,
  FENV_X        = 2,
  FENV_GRAD     = 3
};

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

  luaL_getmetatable (L, GS_METATABLE(GS_FDFMULTIMIN));
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
  return luaL_checkudata (L, index, GS_METATABLE(GS_FDFMULTIMIN));
}

static struct fdfmultimin *
check_init_fdf_multimin (lua_State *L, int index)
{
  struct fdfmultimin *m = luaL_checkudata (L, index, GS_METATABLE(GS_FDFMULTIMIN));
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

  lua_pushcfunction (L, gradient_auto_check);
  mlua_fenv_get (L, 1, FENV_FUNCTION);
  lua_pushvalue (L, 2);
  lua_pushnumber (L, step_size);

  lua_call (L, 3, 0);

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
