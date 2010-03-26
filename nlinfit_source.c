
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

struct params {
  lua_State *L;
  size_t n;
};

struct solver {
  gsl_multifit_fdfsolver *base;
  gsl_multifit_function_fdf fdf[1];
  struct params p[1];
};

static int
FUNCTION (solver, fdf_hook) (const gsl_vector * x, void * params,
			     gsl_vector * f, gsl_matrix * J);

static int
FUNCTION (solver, f_hook)   (const gsl_vector * x, void * params,
			     gsl_vector * f);

static int
FUNCTION (solver, df_hook)  (const gsl_vector * x, void * params,
			     gsl_matrix * J);

#define NLINFIT_MAX_ITER 20

struct solver *
FUNCTION (solver, push) (lua_State *L, size_t n, size_t p, 
			 int findex)
{
  gsl_multifit_fdfsolver_type const * const T = gsl_multifit_fdfsolver_lmsder;
  struct solver *s = lua_newuserdata (L, sizeof (struct solver));
 
  s->base = gsl_multifit_fdfsolver_alloc (T, n, p);

  if (s->base == NULL)
    luaL_error (L, OUT_OF_MEMORY_MSG);

  luaL_getmetatable (L, GS_TYPENAME(NLINFIT));
  lua_setmetatable (L, -2);

  lua_newtable (L);

  lua_pushvalue (L, findex);
  lua_rawseti (L, -2, 1); // 1 <- fdf lua function

  matrix_push_raw (L, p, 1);
  lua_rawseti (L, -2, 2); // 2 <- x

  FUNCTION (matrix, push_view) (L, NULL);
  lua_rawseti (L, -2, 3); // 3 <- f

  FUNCTION (matrix, push_view) (L, NULL);
  lua_rawseti (L, -2, 4); // 4 <- f

  /* if multiplicity is 1 this is not really needed */
  matrix_push_raw (L, n, p);
  lua_rawseti (L, -2, 5); // 5 <- Jbuffer

  lua_setfenv (L, -2);

  return s;
}

static void
fcall_args_prepare (lua_State *L, int nargs)
{
  int j, index;
  lua_getfenv (L, -1);
  index = lua_gettop (L);
  for (j = 1; j <= nargs; j++)
    lua_rawgeti (L, index, j);
  lua_remove  (L, index);
} 

int
FUNCTION (solver, new) (lua_State *L)
{
  struct solver *s;
  gsl_vector_view x0;
  size_t n, nreal, p;
  int findex;

  luaL_checktype (L, 1, LUA_TTABLE);

  solver_get_n_and_p (L, &n, &p);
  nreal = n * MULTIPLICITY;

  solver_get_x0 (L, &x0, p);

  lua_settop (L, 1);
  lua_getfield (L, 1, "fdf");
  findex = 2;

  s = FUNCTION (solver, push) (L, nreal, p, findex);
  lua_replace (L, 1);
  lua_settop (L, 1);

  s->p->L = L;
  s->p->n = n;

  s->fdf->f      = & FUNCTION (solver, f_hook);
  s->fdf->df     = & FUNCTION (solver, df_hook);
  s->fdf->fdf    = & FUNCTION (solver, fdf_hook);
  s->fdf->n      = nreal;
  s->fdf->p      = p;
  s->fdf->params = s->p;

  fcall_args_prepare (L, 5);
  
  gsl_multifit_fdfsolver_set (s->base, s->fdf, & x0.vector);

  lua_pop (L, 5);

  return 1;
}

static struct solver *
FUNCTION (solver, check) (lua_State *L, int index)
{
  return gs_check_userdata (L, index, GS_TYPE(NLINFIT));
}

int
FUNCTION (solver, free) (lua_State *L)
{
  struct solver *s = FUNCTION (solver, check) (L, 1);
  lua_getfenv (L, 1);
  lua_rawgeti (L, -1, 3);
  FUNCTION (matrix, null_view) (L, -1);
  lua_pop (L, 1);
  lua_rawgeti (L, -1, 4);
  FUNCTION (matrix, null_view) (L, -1);
  gsl_multifit_fdfsolver_free (s->base);
  return 0;
}

int
FUNCTION (solver, fdf_hook) (const gsl_vector * x, void * _params, 
			     gsl_vector * f, gsl_matrix * J)
{
  struct params *params = _params;
  lua_State *L = params->L;
  size_t n = params->n, p = x->size;
  size_t nargs = 2;
  gsl_vector_view vv;
  gsl_matrix *xm;
  gsl_matrix *jraw;

  lua_pushvalue (L, 2);

  xm = matrix_check (L, 3);
  vv = gsl_matrix_column (xm, 0);
  gsl_vector_memcpy (&vv.vector, x);
  lua_pushvalue (L, 3);

  if (f)
    FUNCTION (matrix, set_view_and_push) (L, 4, f->data, n, 1, NULL);
  else
    lua_pushnil (L);

  if (J)
    {
      double *jptr;
      if (MULTIPLICITY >= 2)
	{
	  jraw = matrix_check (L, 6);
	  jptr = jraw->data;
	}
      else
	jptr = J->data;

      FUNCTION (matrix, set_view_and_push) (L, 5, jptr, n, p, NULL);
      nargs ++;
    }

  lua_call (L, nargs, 0);

#if MULTIPLICITY >= 2
  if (J)
    {
      double *dst = J->data, *src = jraw->data;
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

int
FUNCTION (solver, iterate) (lua_State *L)
{
  struct solver *s = FUNCTION (solver, check) (L, 1);
  int status;

  mlua_null_cache (L, 1);
  lua_settop (L, 1);

  fcall_args_prepare (L, 5);

  status = gsl_multifit_fdfsolver_iterate (s->base);

  lua_pop (L, 5);

  if (status)
    {
      return luaL_error (L, "error during non-linear fit: %s", 
			 gsl_strerror (status));
    }

  status = gsl_multifit_test_delta (s->base->dx, s->base->x, 1e-4, 1e-4);

  if (status == GSL_CONTINUE)
    lua_pushstring (L, "continue");
  else
    lua_pushstring (L, "success");

  return 1;
}

int
FUNCTION (solver, run) (lua_State *L)
{
  struct solver *s;
  lua_Integer max_iter, iter = 0;
  int iter_status, fit_status;

  s = FUNCTION (solver, check) (L, 1);

  max_iter = (lua_isnumber (L, 2) ? lua_tointeger (L, 2) : NLINFIT_MAX_ITER);

  mlua_null_cache (L, 1);
  lua_settop (L, 1);

  fcall_args_prepare (L, 5);

  do
    {
      iter ++;

      iter_status = gsl_multifit_fdfsolver_iterate (s->base);

      if (iter_status)
	{
	  return luaL_error (L, "error during non-linear fit: %s", 
			     gsl_strerror (iter_status));
	}

      fit_status = gsl_multifit_test_delta (s->base->dx, s->base->x, 1e-4, 1e-4);
    }
  while (fit_status == GSL_CONTINUE && iter < max_iter);

  lua_pop (L, 5);

  return 0;
}

int
FUNCTION (solver, covar) (lua_State *L)
{
  struct solver *s;
  gsl_matrix *covar;
  size_t p;

  s = FUNCTION (solver, check) (L, 1);
  p = MULTIPLICITY * s->fdf->p;
  covar = matrix_push (L, p, p);
  gsl_multifit_covar (s->base->J, 0.0, covar);
  return 1;
}

int
FUNCTION (solver, get_x) (lua_State *L)
{
  struct solver *s = FUNCTION (solver, check) (L, 1);
  gsl_vector *src = s->base->x;
  size_t p = s->fdf->p;
  gsl_matrix *x;

  x = matrix_push (L, p, 1);
  gsl_matrix_set_col (x, 0, src);

  return 1;
}

int
FUNCTION (solver, get_f) (lua_State *L)
{
  struct solver *s = FUNCTION (solver, check) (L, 1);
  size_t n = s->fdf->n / MULTIPLICITY;
  gsl_vector *src = s->base->f;
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
  struct solver *s = FUNCTION (solver, check) (L, 1);
  size_t n = s->fdf->n / MULTIPLICITY, p = s->fdf->p;
  gsl_matrix *src = s->base->J;
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
