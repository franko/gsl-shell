
#include <lua.h>
#include <lauxlib.h>
#include <assert.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit.h>

#include "gs-types.h"
#include "matrix.h"
#include "cmatrix.h"
#include "mlinear.h"

static int      mlinear_fit          (lua_State *L);

static const struct luaL_Reg mlinear_functions[] = {
  {"mlinear",        mlinear_fit},
  {NULL, NULL}
};  

int
mlinear_fit (lua_State *L)
{
  gsl_multifit_linear_workspace *ws;
  gsl_matrix *X = matrix_check (L, 1);
  gsl_matrix *y = matrix_check (L, 2);
  gsl_matrix *w = NULL;
  gsl_vector_const_view yv = gsl_matrix_const_column (y, 0);
  size_t n = X->size1, p = X->size2;
  gsl_matrix *c, *cov;
  gsl_vector_view cv;
  double chisq;
  int status;

  if (!lua_isnil (L, 3))
    w = matrix_check (L, 3);

  if (y->size1 != n || y->size2 != 1)
    return luaL_error (L, "expecting y vector of length %u, got %ux%u matrix",
		       n, y->size1, y->size2);

  if (w != NULL)
    {
      if (w->size1 != n || w->size2 != 1)
	return luaL_error (L, "expecting w vector of length %u, got %ux%u matrix",
			   n, w->size1, w->size2);
    }

  ws = gsl_multifit_linear_alloc (n, p);

  c   = matrix_push_raw (L, p, 1);
  cov = matrix_push_raw (L, p, p);

  cv = gsl_matrix_column (c, 0);

  if (w == NULL)
    status = gsl_multifit_linear (X, &yv.vector, &cv.vector, cov, &chisq, ws);
  else
    {
      gsl_vector_const_view wv = gsl_matrix_const_column (w, 0);
      status = gsl_multifit_wlinear (X, &wv.vector, &yv.vector,
				     &cv.vector, cov, &chisq, ws);
    }

  gsl_multifit_linear_free (ws);

  gs_gsl_errorcheck (L, "multilinear fit", status);

  return 2;
}

void
mlinear_register (lua_State *L)
{
  luaL_register (L, NULL, mlinear_functions);
}
