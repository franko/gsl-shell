
/* fmultimin.c
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

#include "gs-types.h"
#include "multimin.h"
#include "random.h"
#include "lua-utils.h"
#include "matrix.h"

#include <lua.h>
#include <lauxlib.h>

static int    fmultimin_free     (lua_State *L);
static int    fmultimin_set      (lua_State *L);
static int    fmultimin_run      (lua_State *L);
static int    fmultimin_step     (lua_State *L);
static int    fmultimin_index    (lua_State *L);

static int    fmultimin_get_x        (lua_State *L);
static int    fmultimin_get_value    (lua_State *L);

static double fmultimin_f_hook   (const gsl_vector * x, void * p);

struct params {
  lua_State *L;
  size_t n;
  int type_error;
  int args_index;
};

struct fmultimin {
  gsl_multimin_fminimizer *s;
  gsl_multimin_function f[1];
  struct params p[1];
  double size_tol;
};

enum fenvidx {
  FENV_FUNCTION = 1,
  FENV_X        = 2,
};

static const struct luaL_Reg fmultimin_properties[] = {
  {"x",            fmultimin_get_x},
  {"value",        fmultimin_get_value},
  {NULL, NULL}
};

const struct luaL_Reg fmultimin_methods[] = {
  {"__gc",         fmultimin_free},
  {"__index",      fmultimin_index},
  {"step",         fmultimin_step},
  {"run",          fmultimin_run},
  {"set",          fmultimin_set},
  {NULL, NULL}
};

#define FMULTIMIN_MAX_ITER 20

struct fmultimin *
push_new_fmultimin (lua_State *L, int findex, size_t n,
		    const gsl_multimin_fminimizer_type *T)
{
  struct fmultimin *m = lua_newuserdata (L, sizeof (struct fmultimin));
  m->s = gsl_multimin_fminimizer_alloc (T, n);

  if (m->s == NULL)
    luaL_error (L, OUT_OF_MEMORY_MSG);

  luaL_getmetatable (L, GS_METATABLE(GS_FMULTIMIN));
  lua_setmetatable (L, -2);

  lua_newtable (L);

  lua_pushvalue (L, findex);
  lua_rawseti (L, -2, FENV_FUNCTION);

  matrix_push_raw (L, n, 1);
  lua_rawseti (L, -2, FENV_X);

  lua_setfenv (L, -2);

  return m;
}

static struct fmultimin *
check_fmultimin (lua_State *L, int index)
{
  return luaL_checkudata (L, index, GS_METATABLE(GS_FMULTIMIN));
}

static struct fmultimin *
check_init_fmultimin (lua_State *L, int index)
{
  struct fmultimin *m = luaL_checkudata (L, index, GS_METATABLE(GS_FMULTIMIN));
  if (m->f->params == NULL)
    luaL_error (L, "minimizer is not initialised");
  return m;
}

int
fmultimin_new (lua_State *L)
{
  struct fmultimin *m;
  int ni = luaL_checkinteger (L, 2);
  int findex = 1;
  size_t n;

  if (ni <= 0)
    return luaL_error (L, "argument #1 should be a positive integer");
  else
    n = (size_t) ni;

  if (!lua_isfunction (L, findex))
    luaL_typerror (L, findex, "function");

  m = push_new_fmultimin (L, findex, n, gsl_multimin_fminimizer_nmsimplex2);

  m->f->n      = n;
  m->f->f      = fmultimin_f_hook;
  m->f->params = NULL;

  return 1;
}

int
fmultimin_free (lua_State *L)
{
  struct fmultimin *m = check_fmultimin (L, 1);
  gsl_multimin_fminimizer_free (m->s);
  return 0;
}

double
fmultimin_f_hook (const gsl_vector * x, void * _p)
{
  struct params *p = _p;
  lua_State *L = p->L;
  size_t n = p->n, k;
  gsl_matrix *lx;
  int aindex = p->args_index;

  lua_pushvalue (L, aindex);

  lx = matrix_check (L, aindex+1);
  for (k = 0; k < n; k++)
    lx->data[k] = x->data[k];
  lua_pushvalue (L, aindex+1);

  lua_call (L, 1, 1);

  if (lua_isnumber (L, -1))
    return lua_tonumber (L, -1);
  else
    {
      p->type_error = 1;
      return GSL_NAN;
    }

  lua_pop (L, 1);
}

static void
fcall_args_prepare (lua_State *L, struct params *p, int index)
{
  int bindex;
  lua_getfenv (L, index);

  bindex = lua_gettop  (L);
  lua_rawgeti (L, bindex, FENV_FUNCTION);
  lua_rawgeti (L, bindex, FENV_X);
  lua_remove  (L, bindex);

  p->args_index = bindex;
} 

int
fmultimin_set (lua_State *L)
{
  struct fmultimin *m = check_fmultimin (L, 1);
  gsl_matrix *x0m = matrix_check (L, 2);
  gsl_matrix *stepm = matrix_check (L, 3);
  double size_tol = gs_check_number (L, 4, FP_CHECK_NORMAL);
  gsl_vector_view x0v, stepv;
  size_t n = m->f->n;
  int status;

  if (x0m->size2 > 1)
    return gs_type_error (L, 2, "column vector");

  if (stepm->size2 > 1)
    return gs_type_error (L, 2, "column vector");

  x0v =   gsl_matrix_column (x0m,   0);
  stepv = gsl_matrix_column (stepm, 0);

  m->f->params = m->p;
  m->size_tol = pow (size_tol, n);

  m->p->L = L;
  m->p->n = n;
  m->p->type_error = 0;

  lua_settop (L, 2);  /* get rid of the step_size */

  mlua_null_cache (L, 1);
  fcall_args_prepare (L, m->p, 1);

  status = gsl_multimin_fminimizer_set (m->s, m->f, &x0v.vector, &stepv.vector);

  if (status != GSL_SUCCESS)
    return luaL_error (L, "minimizer:set %s", gsl_strerror (status));

  return 0;
}
    
int
fmultimin_step (lua_State *L)
{
  struct fmultimin *m = check_init_fmultimin (L, 1);
  double size;
  int status;

  mlua_null_cache (L, 1);
  fcall_args_prepare (L, m->p, 1);

  status = gsl_multimin_fminimizer_iterate (m->s);

  if (status != GSL_SUCCESS)
    return luaL_error (L, "minimizer:step %s", gsl_strerror (status));

  size = gsl_multimin_fminimizer_size (m->s);
  status = gsl_multimin_test_size (size, m->size_tol);

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
fmultimin_run (lua_State *L)
{
  struct fmultimin *m = check_init_fmultimin (L, 1);
  size_t iter = 0;
  double size;
  int status;

  mlua_null_cache (L, 1);
  fcall_args_prepare (L, m->p, 1);

  do {
    iter++;

    status = gsl_multimin_fminimizer_iterate (m->s);

    if (status)
      break;

    size = gsl_multimin_fminimizer_size (m->s);
    status = gsl_multimin_test_size (size, m->size_tol);

    if (status == GSL_SUCCESS)
      return 0;
  }
  while (status == GSL_CONTINUE && iter < FMULTIMIN_MAX_ITER);

  if (m->p->type_error)
    return luaL_error (L, "function should return a real number");

  return luaL_error (L, "minimizer:run %s", gsl_strerror (status));
}

int
fmultimin_get_x (lua_State *L)
{
  struct fmultimin *m = check_init_fmultimin (L, 1);
  gsl_matrix *xm = matrix_push (L, m->p->n, 1);
  gsl_vector *x = gsl_multimin_fminimizer_x (m->s);
  size_t k;
  for (k = 0; k < m->p->n; k++)
    xm->data[k] = x->data[k];
  return 1;
}

int
fmultimin_get_value (lua_State *L)
{
  struct fmultimin *m = check_init_fmultimin (L, 1);
  double v = gsl_multimin_fminimizer_minimum (m->s);
  lua_pushnumber (L, v);
  return 1;
}

int
fmultimin_index (lua_State *L)
{
  return mlua_index_with_properties (L, fmultimin_properties, true);
}
