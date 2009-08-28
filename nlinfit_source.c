
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

int
FUNCTION (solver, new) (lua_State *L)
{
  gsl_multifit_fdfsolver_type const * const T = gsl_multifit_fdfsolver_lmsder;
  gsl_multifit_fdfsolver * s;
  struct fdfsolver *sext;
  size_t n, p;
  int nb;

  nb = luaL_checkinteger (L, 1);
  luaL_argcheck (L, nb > 0, 1, "the number of observations should be > 0");
  n = nb * MULTIPLICITY;

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
  sext->fit_data->n = n / MULTIPLICITY;
  sext->fit_data->x = gsl_vector_alloc (p);
#if MULTIPLICITY >= 2
  sext->fit_data->j_raw = gsl_vector_alloc (n * p);
#else
  sext->fit_data->j_raw = NULL;
#endif

  sext->base_type = BASE_TYPE;

  /* set engine.solver */
  lua_setfield (L, -2, "solver");

  luaL_getmetatable (L, TYPE (name_solver));
  lua_setmetatable (L, -2);

  /* return the fit engine table */
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

static void
null_matrix_view (lua_State *L, int index)
{
  VIEW (gsl_matrix) *m = FUNCTION (matrix, check_view) (L, index);
  m->matrix.data = NULL;
}

static void
null_fdf_arguments (lua_State *L, int index)
{
  gsl_matrix_view *m = matrix_check_view (L, index);
  m->matrix.data = NULL;
  null_matrix_view (L, index+1);
  null_matrix_view (L, index+2);
}

static void
push_fdf_arguments (lua_State *L)
{
  matrix_push_view (L, NULL);
  FUNCTION (matrix, push_view) (L, NULL);
  FUNCTION (matrix, push_view) (L, NULL);
}

int
FUNCTION (solver, set) (lua_State *L)
{
  struct fdfsolver *sext; 
  struct {
    gsl_matrix *matrix;
    gsl_vector_view view;
  } x0;

  sext = FUNCTION (solver, check) (L, 1);

  x0.matrix = matrix_check (L, 3);
  if (x0.matrix->size2 != 1)
    luaL_typerror (L, 3, "vector");
  x0.view = gsl_matrix_column (x0.matrix, 0);

  lua_pop (L, 1);

  sext->fdf->f      = & FUNCTION (solver, f_hook);
  sext->fdf->df     = & FUNCTION (solver, df_hook);
  sext->fdf->fdf    = & FUNCTION (solver, fdf_hook);
  /* sext->fdf->n already ok */
  /* sext->fdf->p already ok */
  sext->fdf->params = sext->fit_data;

  push_fdf_arguments (L);
  
  gsl_multifit_fdfsolver_set (sext->base, sext->fdf, & x0.view.vector);

  null_fdf_arguments (L, 3);

  lua_pop (L, 3);

  lua_setfield (L, 1, "fdf");

  return 0;
}

static void
FUNCTION (set_matrix, view_and_push) (lua_State *L, int index, 
				      double *data, size_t n1, size_t n2)
{
  VIEW (gsl_matrix) *view = FUNCTION (matrix, check_view) (L, index);
  *view = FUNCTION (gsl_matrix, view_array) (data, n1, n2);
  lua_pushvalue (L, index);
}

#if MULTIPLICITY >= 2
static void
copy_jacobian (double *cmpl, double *real, size_t n, size_t p,
	       size_t multiplicity, bool inverse)
{
  gsl_vector_view dview, sview;
  double *cp, *rp;
  size_t k, nu;

  for (nu = 0; nu < multiplicity; nu++)
    {
      cp = cmpl + nu;
      rp = real + p*nu;
      for (k = 0; k < p; k++, rp += 1, cp += multiplicity)
	{
	  dview = gsl_vector_view_array_with_stride (cp, multiplicity * p, n);
	  sview = gsl_vector_view_array_with_stride (rp, multiplicity * p, n);
	  if (inverse)
	    gsl_vector_memcpy (& sview.vector, & dview.vector);
	  else
	    gsl_vector_memcpy (& dview.vector, & sview.vector);
	}
    }
}
#endif

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

  //  FUNCTION (set_matrix, view_and_push) (L, 3, params->x->data, p, 1);

  if (f)
    FUNCTION (set_matrix, view_and_push) (L, 4, f->data, n, 1);
  else
    lua_pushnil (L);

  if (J)
    {
      double *jptr = (MULTIPLICITY >= 2 ? params->j_raw->data : J->data);
      FUNCTION (set_matrix, view_and_push) (L, 5, jptr, n, p);
    }

  lua_call (L, nargs, 0);

#if MULTIPLICITY >= 2
  if (J)
    copy_jacobian (params->j_raw->data, J->data, n, p, MULTIPLICITY, true);
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

#if MULTIPLICITY >= 2
  copy_jacobian (m->data, src->data, n, p, MULTIPLICITY, false);
#else
  gsl_matrix_memcpy (m, src);
#endif

  return 1;
}

int
FUNCTION (solver, index) (lua_State *L)
{
  char const * key;
  const struct luaL_Reg *reg;

  key = lua_tostring (L, 2);
  if (key == NULL)
    return 0;

  reg = mlua_find_method (FUNCTION (solver, properties), key);
  if (reg)
    {
      return mlua_get_property (L, reg, true);
    }
  
  reg = mlua_find_method (FUNCTION (solver, methods), key);
  if (reg)
    {
      lua_pushcfunction (L, reg->func);
      return 1;
    }

  return 0;
}
