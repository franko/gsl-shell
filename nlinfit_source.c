
/* nlinfit_source.c
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

#define NLINFIT_MAX_ITER 20

static int
FUNCTION (solver, fdf_hook) (const gsl_vector * x, void * params,
			     gsl_vector * f, gsl_matrix * J);

static int
FUNCTION (solver, f_hook)   (const gsl_vector * x, void * params,
			     gsl_vector * f);

static int
FUNCTION (solver, df_hook)  (const gsl_vector * x, void * params,
			     gsl_matrix * J);

static void
FUNCTION (solver, push) (lua_State *L)
{
  lua_newtable (L);
  luaL_getmetatable (L, TYPE (name_solver));
  lua_setmetatable (L, -2);
}

static void
null_fdf_arguments (lua_State *L, int index)
{
  matrix_null_view (L, index);
  FUNCTION (matrix, null_view) (L, index+1);
  FUNCTION (matrix, null_view) (L, index+2);
}

static void
push_fdf_arguments (lua_State *L)
{
  matrix_push_view (L, NULL);
  FUNCTION (matrix, push_view) (L, NULL);
  FUNCTION (matrix, push_view) (L, NULL);
}

int
FUNCTION (solver, new) (lua_State *L)
{
  struct fdfsolver *sext;
  gsl_vector_view x0;
  size_t n, nreal, p;

  luaL_checktype (L, 1, LUA_TTABLE);

  solver_get_n_and_p (L, &n, &p);
  nreal = n * MULTIPLICITY;

  solver_get_x0 (L, &x0, p);

  /* main table to store the fit engine, i.e. the solver and the 
     accessory data */
  FUNCTION (solver, push) (L);

  sext = push_new_fdfsolver (L, nreal, p);

  sext->fit_data->L = L;
  sext->fit_data->n = n;
  sext->fit_data->x = gsl_vector_alloc (p);
#if MULTIPLICITY >= 2
  sext->fit_data->j_raw = gsl_vector_alloc (nreal * p);
#else
  sext->fit_data->j_raw = NULL;
#endif

  sext->fdf->f      = & FUNCTION (solver, f_hook);
  sext->fdf->df     = & FUNCTION (solver, df_hook);
  sext->fdf->fdf    = & FUNCTION (solver, fdf_hook);
  sext->fdf->n      = nreal;
  sext->fdf->p      = p;
  sext->fdf->params = sext->fit_data;

  sext->base_type = BASE_TYPE;

  /* set engine.solver */
  lua_setfield (L, -2, "solver");

  lua_getfield (L, 1, "fdf");

  /* we get rid of the constructor spec */
  lua_remove (L, 1);

  push_fdf_arguments (L);
  
  gsl_multifit_fdfsolver_set (sext->base, sext->fdf, & x0.vector);

  null_fdf_arguments (L, 3);
  lua_pop (L, 3);

  lua_setfield (L, 1, "fdf");

  return 1;
}

static struct fdfsolver *
FUNCTION (solver, check) (lua_State *L, int index)
{
  struct fdfsolver *sext;

  luaL_checktype (L, index, LUA_TTABLE);

  lua_getfield (L, index, "solver");
  sext = check_fdfsolver (L, -1);
  lua_pop (L, 1);

  if (sext->base_type != BASE_TYPE)
    luaL_error (L, "expected %s type solver",  math_name[BASE_TYPE]);

  return sext;
}

int
FUNCTION (solver, fdf_hook) (const gsl_vector * x, void * _params, 
			     gsl_vector * f, gsl_matrix * J)
{
  struct fit_data *params = _params;
  lua_State *L = params->L;
  size_t n = params->n, p = x->size;
  size_t nargs = (J ? 3 : 2);
  gsl_matrix_view *xview;

  lua_pushvalue (L, 2);

  gsl_vector_memcpy (params->x, x);

  xview = matrix_check_view (L, 3);
  *xview = gsl_matrix_view_array (params->x->data, p, 1);
  lua_pushvalue (L, 3);

  if (f)
    FUNCTION (matrix, set_view_and_push) (L, 4, f->data, n, 1, NULL);
  else
    lua_pushnil (L);

  if (J)
    {
      double *jptr = (MULTIPLICITY >= 2 ? params->j_raw->data : J->data);
      FUNCTION (matrix, set_view_and_push) (L, 5, jptr, n, p, NULL);
    }

  lua_call (L, nargs, 0);

#if MULTIPLICITY >= 2
  if (J)
    {
      double *dst = J->data, *src = params->j_raw->data;
      FUNCTION (matrix, jacob_copy_cmpl_to_real) (dst, src, n, p, MULTIPLICITY);
    }
#endif

  return GSL_SUCCESS;
}

int
FUNCTION (solver, f_hook) (const gsl_vector * x, void * params, gsl_vector * f)
{
  return FUNCTION (solver, fdf_hook) (x, params, f, NULL);
}

int
FUNCTION (solver, df_hook) (const gsl_vector * x, void * params, gsl_matrix * J)
{
  return FUNCTION (solver, fdf_hook) (x, params, NULL, J);
}

static struct fdfsolver *
FUNCTION (solver, check_defined) (lua_State *L, int index)
{
  struct fdfsolver *sext; 
  sext = FUNCTION (solver, check) (L, 1);
  if (sext->fdf->fdf == NULL)
    luaL_error (L, "solver fdf function not initialised");
  return sext;
}

int
FUNCTION (solver, iterate) (lua_State *L)
{
  struct fdfsolver *sext; 
  int status;

  sext = FUNCTION (solver, check_defined) (L, 1);

  mlua_null_cache (L, 1);
  lua_settop (L, 1);

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
FUNCTION (solver, run) (lua_State *L)
{
  struct fdfsolver *sext; 
  lua_Integer max_iter, iter = 0;
  int iter_status, fit_status;

  sext = FUNCTION (solver, check_defined) (L, 1);

  max_iter = (lua_isnumber (L, 2) ? lua_tointeger (L, 2) : NLINFIT_MAX_ITER);

  mlua_null_cache (L, 1);
  lua_settop (L, 1);

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
FUNCTION (solver, covar) (lua_State *L)
{
  struct fdfsolver *sext;
  gsl_matrix *covar;
  size_t p;

  sext = FUNCTION (solver, check_defined) (L, 1);
  p = MULTIPLICITY * sext->fdf->p;
  covar = matrix_push (L, p, p);
  gsl_multifit_covar (sext->base->J, 0.0, covar);
  return 1;
}

int
FUNCTION (solver, get_x) (lua_State *L)
{
  struct fdfsolver *sext = FUNCTION (solver, check_defined) (L, 1);
  gsl_vector *src = sext->base->x;
  size_t p = sext->fdf->p;
  gsl_matrix *x;

  x = matrix_push (L, p, 1);
  gsl_matrix_set_col (x, 0, src);

  return 1;
}

int
FUNCTION (solver, get_f) (lua_State *L)
{
  struct fdfsolver *sext = FUNCTION (solver, check_defined) (L, 1);
  size_t n = sext->fdf->n / MULTIPLICITY;
  gsl_vector *src = sext->base->f;
  TYPE (gsl_matrix) *f;
  VIEW (gsl_matrix) fview;

  f = FUNCTION (matrix, push) (L, n, 1);
  fview = FUNCTION (gsl_matrix, view_array) (src->data, n, 1);
  FUNCTION (gsl_matrix, memcpy) (f, & fview.matrix);
  return 1;
}

int
FUNCTION (solver, get_jacob) (lua_State *L)
{
  struct fdfsolver *sext = FUNCTION (solver, check_defined) (L, 1);
  size_t n = sext->fdf->n / MULTIPLICITY, p = sext->fdf->p;
  gsl_matrix *src = sext->base->J;
  TYPE (gsl_matrix) *m;

  m = FUNCTION (matrix, push) (L, n, p);

  FUNCTION (matrix, jacob_copy_real_to_cmpl) (m->data, src->data, n, p,
					      MULTIPLICITY);

  return 1;
}

int
FUNCTION (solver, index) (lua_State *L)
{
  return mlua_index_with_properties (L, FUNCTION (solver, properties), true);
}
