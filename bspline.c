
#include <lua.h>
#include <lauxlib.h>
#include <assert.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_bspline.h>

#include "bspline.h"
#include "gs-types.h"
#include "matrix.h"

struct bspline {
  gsl_bspline_workspace *ws;
  size_t k;
  size_t nbreak;
};

static int         bspline_new              (lua_State *L);
static int         bspline_free             (lua_State *L);
static int         bspline_eval             (lua_State *L);
static int         bspline_model     (lua_State *L);

static const struct luaL_Reg bspline_methods[] = {
  {"__gc",          bspline_free},
  {"model",         bspline_model}, 
  {"eval",          bspline_eval},
  {NULL, NULL}
};

static const struct luaL_Reg bspline_functions[] = {
  {"bspline",         bspline_new},
  {NULL, NULL}
};  

int
bspline_new (lua_State *L)
{
  gsl_matrix *breaks = NULL;
  gsl_vector_view brk;
  double a, b;
  int nbreak;
  struct bspline *bs;
  int nargs = 3;
  int k = 4;

  if (lua_isnumber (L, 1))
    {
      a = lua_tonumber (L, 1);
      b = luaL_checknumber (L, 2);
      nbreak = luaL_checkinteger (L, 3);
    }
  else
    {
      breaks = matrix_check (L, 1);
      brk = gsl_matrix_column (breaks, 0);
      nbreak = brk.vector.size;
      nargs = 1;
    }

  if (nbreak <= 2)
    return luaL_error (L, "number of knots should be >= 2");

  if (lua_gettop (L) >= nargs+1 && !lua_isnil (L, nargs+1))
    {
      k = luaL_checkinteger (L, nargs+1);
      if (k <= 0)
	return luaL_error (L, "bspline order should be > 0");
    }

  bs = lua_newuserdata (L, sizeof(struct bspline));
  bs->ws = gsl_bspline_alloc ((size_t) k, (size_t) nbreak);
  bs->k = k;
  bs->nbreak = (size_t) nbreak;

  if (breaks)
    gsl_bspline_knots (&brk.vector, bs->ws);
  else
    gsl_bspline_knots_uniform (a, b, bs->ws);

  gs_set_metatable (L, GS_BSPLINE);

  return 1;
}

static struct bspline *
bspline_check (lua_State *L, int index)
{
  return gs_check_userdata (L, index, GS_BSPLINE);
}

int
bspline_free (lua_State *L)
{
  struct bspline *bs = bspline_check (L, 1);
  gsl_bspline_free (bs->ws);
  return 0;
}

int
bspline_model (lua_State *L)
{
  struct bspline *bs = bspline_check (L, 1);
  gsl_matrix *x = matrix_check (L, 2);
  size_t j, n = x->size1;
  gsl_matrix *M;

  M = matrix_push_raw (L, n, bs->nbreak + bs->k - 2);

  for (j = 0; j < n; j++)
    {
      double xj = gsl_matrix_get (x, j, 0);
      gsl_vector_view B = gsl_matrix_row (M, j);
      gsl_bspline_eval(xj, &B.vector, bs->ws);
    }

  return 1;
}

int
bspline_eval (lua_State *L)
{
  struct bspline *bs = bspline_check (L, 1);
  double x = luaL_checknumber (L, 2);
  gsl_vector_view Bv;
  gsl_matrix *B;

  B = matrix_push_raw (L, bs->nbreak + bs->k - 2, 1);

  Bv = gsl_matrix_column (B, 0);
  gsl_bspline_eval(x, &Bv.vector, bs->ws);

  return 1;
}

void
bspline_register (lua_State *L)
{
  luaL_newmetatable (L, GS_METATABLE(GS_BSPLINE));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, bspline_methods);
  lua_pop (L, 1);

  luaL_register (L, NULL, bspline_functions);
}
