
/* nlinfit.c
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

#include "matrix.h"
#include "lua-utils.h"

struct fit_data {
  lua_State *L;
  gsl_vector *x;
};

struct fdfsolver {
  gsl_multifit_fdfsolver *base;
  gsl_multifit_function_fdf fdf[1];
  struct fit_data fit_data[1];
};

#define NLINFIT_MAX_ITER 30

#define check_fdfsolver(L, n) (struct fdfsolver *) luaL_checkudata (L, n, fdfsolver_mt_name)

static char const * const fdfsolver_mt_name     = "GSL.fdfsolver";
static char const * const solver_mt_name        = "GSL.solver";

extern int  luaopen_gsl              (lua_State *L);

/* declaration of lua function for solver methods */
static int solver_new                (lua_State *);
static int solver_set                (lua_State *);
static int solver_run                (lua_State *);
static int solver_iterate            (lua_State *);
static int solver_index              (lua_State *);
static int solver_covar              (lua_State *);
static int solver_get_x              (lua_State *);
static int solver_get_f              (lua_State *);
static int solver_get_jacob          (lua_State *);

static int fdfsolver_dealloc         (lua_State *);

static int
solver_fdf_hook (const gsl_vector * x, void * params, gsl_vector * f,
		 gsl_matrix * J);

static int
solver_f_hook (const gsl_vector * x, void * params, gsl_vector * f);

static int
solver_df_hook (const gsl_vector * x, void * params, gsl_matrix * J);

static const struct luaL_Reg fdfsolver_methods[] = {
  {"__gc",          fdfsolver_dealloc},
  {NULL, NULL}
};

static const struct luaL_Reg solver_methods[] = {
  {"set",          solver_set},
  {"iterate",      solver_iterate},
  {"run",          solver_run},
  {NULL, NULL}
};  

static const struct luaL_Reg solver_properties[] = {
  {"covar",        solver_covar},
  {"x",            solver_get_x},
  {"f",            solver_get_f},
  {"J",            solver_get_jacob},
  {NULL, NULL}
};

static const struct luaL_Reg solver_functions[] = {
  {"solver",      solver_new},
  {NULL, NULL}
};

int
fdfsolver_dealloc (lua_State *L)
{
  struct fdfsolver *fdf = check_fdfsolver (L, 1);
  gsl_multifit_fdfsolver_free (fdf->base);
  gsl_vector_free (fdf->fit_data->x);
  return 0;
}

int
solver_new (lua_State *L)
{
  gsl_multifit_fdfsolver_type const * const T = gsl_multifit_fdfsolver_lmsder;
  gsl_multifit_fdfsolver * s;
  struct fdfsolver *sext;
  size_t n, p;
  int nb;

  nb = luaL_checkinteger (L, 1);
  luaL_argcheck (L, nb > 0, 1, "the number of observations should be > 0");
  n = (size_t) nb;

  nb = luaL_checkinteger (L, 2);
  luaL_argcheck (L, nb > 0, 1, "the number of parameters should be > 0");
  p = (size_t) nb;

  /* main table to store the fit engine, i.e. the solver and the 
     accessory data */
  lua_newtable (L);

  if (n < p)
    return luaL_error (L, "insufficient data points, n < p");

  sext = lua_newuserdata (L, sizeof (struct fdfsolver));

  s = gsl_multifit_fdfsolver_alloc (T, n, p);
  if (s == NULL)
    return luaL_error (L, OUT_OF_MEMORY_MSG);

  sext->base = s;

  /* when the following pointer is NULL it means that the
     fit engine function is still not defined */
  sext->fdf->fdf = NULL;
  sext->fdf->n = n;
  sext->fdf->p = p;

  luaL_getmetatable (L, fdfsolver_mt_name);
  lua_setmetatable (L, -2);

  sext->fit_data->L = L;
  sext->fit_data->x = gsl_vector_alloc (p);

  /* set engine.solver */
  lua_setfield (L, -2, "solver");

  luaL_getmetatable (L, solver_mt_name);
  lua_setmetatable (L, -2);

  /* return the fit engine table */
  return 1;
}

static void
set_vector_view_and_push (lua_State *L, int index, gsl_vector *v)
{
  gsl_matrix_view *mview = check_matrix_view (L, index);
  *mview = gsl_matrix_view_array (v->data, v->size, 1);
  lua_pushvalue (L, index);
}

static void
set_matrix_view_and_push (lua_State *L, int index, gsl_matrix *m)
{
  gsl_matrix_view *mview = check_matrix_view (L, index);
  *mview = gsl_matrix_view_array (m->data, m->size1, m->size2);
  lua_pushvalue (L, index);
}

int
solver_fdf_hook (const gsl_vector * x, void * params, gsl_vector * f,
		 gsl_matrix * J)
{
  struct fit_data *data = params;
  lua_State *L = data->L;

  lua_pushvalue (L, 2);

  gsl_vector_memcpy (data->x, x);

  set_vector_view_and_push (L, 3, data->x);
  set_vector_view_and_push (L, 4, f);
  set_matrix_view_and_push (L, 5, J);

  lua_call (L, 3, 0);

  return GSL_SUCCESS;
}

int
solver_f_hook (const gsl_vector * x, void * params, gsl_vector * f)
{
  struct fit_data *data = params;
  lua_State *L = data->L;

  lua_pushvalue (L, 2);

  gsl_vector_memcpy (data->x, x);

  set_vector_view_and_push (L, 3, data->x);
  set_vector_view_and_push (L, 4, f);

  lua_call (L, 2, 0);

  return GSL_SUCCESS;
}

int
solver_df_hook (const gsl_vector * x, void * params, gsl_matrix * J)
{
  struct fit_data *data = params;
  lua_State *L = data->L;

  lua_pushvalue (L, 2);

  gsl_vector_memcpy (data->x, x);

  set_vector_view_and_push (L, 3, data->x);
  lua_pushnil (L);
  set_matrix_view_and_push (L, 5, J);

  lua_call (L, 3, 0);

  return GSL_SUCCESS;
}

static struct fdfsolver *
c_solver_check_args (lua_State *L, bool check_init)
{
  struct fdfsolver *sext;

  luaL_checktype (L, 1, LUA_TTABLE);

  lua_getfield (L, 1, "solver");
  sext = check_fdfsolver (L, -1);
  lua_pop (L, 1);

  if (check_init)
    {
      if (sext->fdf->fdf == NULL)
	luaL_error (L, "fit function not defined");
    }

  return sext;
}

static void
null_matrix_view (lua_State *L, int index)
{
  gsl_matrix_view *mview = check_matrix_view (L, index);
  mview->matrix.data = NULL;
}

static void
null_fdf_arguments (lua_State *L, int index)
{
  null_matrix_view (L, index);
  null_matrix_view (L, index+1);
  null_matrix_view (L, index+2);
}

static void
push_fdf_arguments (lua_State *L)
{
  push_matrix_view (L, NULL);
  push_matrix_view (L, NULL);
  push_matrix_view (L, NULL);
}

int
solver_set (lua_State *L)
{
  struct fdfsolver *sext; 
  gsl_matrix *x0;
  gsl_vector_view x0view;

  sext = c_solver_check_args (L, false);

  x0 = check_matrix (L, 3);
  if (x0->size2 != 1)
    luaL_typerror (L, 3, "vector");
  x0view = gsl_matrix_column (x0, 0);
  lua_pop (L, 1);

  sext->fdf->f      = & solver_f_hook;
  sext->fdf->df     = & solver_df_hook;
  sext->fdf->fdf    = & solver_fdf_hook;
  /* sext->fdf->n already ok */
  /* sext->fdf->p already ok */
  sext->fdf->params = sext->fit_data;

  push_fdf_arguments (L);
  
  gsl_multifit_fdfsolver_set (sext->base, sext->fdf, & x0view.vector);

  null_fdf_arguments (L, 3);

  lua_pop (L, 3);

  lua_setfield (L, 1, "fdf");

  return 0;
}

int
solver_iterate (lua_State *L)
{
  struct fdfsolver *sext; 
  int status;

  sext = c_solver_check_args (L, true);

  mlua_null_cache (L, 1);
  lua_getfield (L, 1, "fdf");

  push_fdf_arguments (L);

  status = gsl_multifit_fdfsolver_iterate (sext->base);

  null_fdf_arguments (L, 3);

  lua_pop (L, 4);

  if (status)
    {
      return luaL_error (L, "error during non-linear fit: %s", 
			 gsl_strerror (status));
    }

  status = gsl_multifit_test_delta (sext->base->dx, sext->base->x, 1e-4, 1e-4);

  if (status == GSL_CONTINUE)
    lua_pushstring (L, "continue");
  else
    lua_pushstring (L, "terminated");

  return 1;
}

int
solver_run (lua_State *L)
{
  struct fdfsolver *sext; 
  lua_Integer max_iter, iter = 0;
  int iter_status, fit_status;

  sext = c_solver_check_args (L, true);

  max_iter = (lua_isnumber (L, 2) ? lua_tointeger (L, 2) : NLINFIT_MAX_ITER);

  mlua_null_cache (L, 1);
  lua_getfield (L, 1, "fdf");

  push_fdf_arguments (L);

  do
    {
      iter ++;

      iter_status = gsl_multifit_fdfsolver_iterate (sext->base);

      if (iter_status)
	{
	  return luaL_error (L, "error during non-linear fit: %s", 
			     gsl_strerror (iter_status));
	}

      fit_status = gsl_multifit_test_delta (sext->base->dx, sext->base->x,
					    1e-4, 1e-4);
    }
  while (fit_status == GSL_CONTINUE && iter < max_iter);

  null_fdf_arguments (L, 3);

  lua_pop (L, 4);

  return 0;
}

int
solver_covar (lua_State *L)
{
  struct fdfsolver *sext;
  gsl_matrix *covar;
  size_t p;

  sext = c_solver_check_args (L, true);
  p = sext->fdf->p;
  covar = push_matrix (L, p, p);
  gsl_multifit_covar (sext->base->J, 0.0, covar);
  return 1;
}

int
solver_get_x (lua_State *L)
{
  struct fdfsolver *sext;
  gsl_matrix *x;
  sext = c_solver_check_args (L, true);
  x = push_matrix (L, sext->fdf->p, 1);
  assert (x->size1 == sext->base->x->size);
  gsl_matrix_set_col (x, 0, sext->base->x);
  return 1;
}

int
solver_get_f (lua_State *L)
{
  struct fdfsolver *sext;
  gsl_matrix *f;
  sext = c_solver_check_args (L, true);
  f = push_matrix (L, sext->fdf->n, 1);
  assert (f->size1 == sext->base->f->size);
  gsl_matrix_set_col (f, 0, sext->base->f);
  return 1;
}

int
solver_get_jacob (lua_State *L)
{
  struct fdfsolver *sext;
  gsl_matrix *m;
  sext = c_solver_check_args (L, true);
  m = push_matrix (L, sext->fdf->n, sext->fdf->p);
  assert (m->size1 == sext->base->J->size1 && m->size2 == sext->base->J->size2);
  gsl_matrix_memcpy (m, sext->base->J);
  return 1;
}

int
solver_index (lua_State *L)
{
  char const * key;
  const struct luaL_Reg *reg;

  key = lua_tostring (L, 2);
  if (key == NULL)
    return 0;

  reg = mlua_find_method (solver_properties, key);
  if (reg)
    {
      return mlua_get_property (L, reg, true);
    }
  
  reg = mlua_find_method (solver_methods, key);
  if (reg)
    {
      lua_pushcfunction (L, reg->func);
      return 1;
    }

  return 0;
}

void
solver_register (lua_State *L)
{
  /* fdfsolver declaration */
  luaL_newmetatable (L, fdfsolver_mt_name);
  luaL_register (L, NULL, fdfsolver_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, solver_mt_name);
  lua_pushcfunction (L, solver_index);
  lua_setfield (L, -2, "__index");
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, solver_functions);
}
