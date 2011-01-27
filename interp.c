#include <lua.h>
#include <lauxlib.h>
#include <assert.h>
#include <string.h>
#include <gsl/gsl_interp.h>

#include "interp.h"
#include "gs-types.h"
#include "matrix.h"

struct interp {
  gsl_interp *interp;
  gsl_interp_accel *acc;
  const double *xsrc, *ysrc;
  int n;
};

enum fenv_pos {
  FENV_X_VECTOR = 1,
  FENV_Y_VECTOR = 2,
};

static int          interp_new       (lua_State *L);
static int          interp_free      (lua_State *L);
static int          interp_eval      (lua_State *L);
static int          interp_deriv     (lua_State *L);
static int          interp_deriv2    (lua_State *L);
static int          interp_integ     (lua_State *L);

static const struct luaL_Reg interp_methods[] = {
  {"__gc",          interp_free},
  {"eval",          interp_eval},
  {"deriv",         interp_deriv},
  {"deriv2",        interp_deriv2},
  {"integ",         interp_integ},
  {NULL, NULL}
};

static const struct luaL_Reg interp_functions[] = {
  {"interp",         interp_new},
  {NULL, NULL}
};  

static const gsl_interp_type *
interp_algo_lookup (const char *req_name)
{
  if (strcmp ("linear", req_name) == 0)
    return gsl_interp_linear;
  else if (strcmp ("polynomial", req_name) == 0)
    return gsl_interp_polynomial;
  else if (strcmp ("cspline", req_name) == 0)
    return gsl_interp_cspline;
  else if (strcmp ("cspline_periodic", req_name) == 0)
    return gsl_interp_cspline_periodic;
  else if (strcmp ("akima", req_name) == 0)
    return gsl_interp_akima;
  else if (strcmp ("akima_periodic", req_name) == 0)
    return gsl_interp_akima_periodic;

  return NULL;
}

static void
interp_set_ref (lua_State *L, int index_x, int index_y)
{
  /*  INDEX_SET_ABS_2(L, index_x, index_y); */
  lua_newtable (L);
  lua_pushvalue (L, index_x);
  lua_rawseti (L, -2, FENV_X_VECTOR);
  lua_pushvalue (L, index_y);
  lua_rawseti (L, -2, FENV_Y_VECTOR);
  lua_setfenv (L, -2);
}

int
interp_new (lua_State *L)
{
  gsl_matrix *x = matrix_check (L, 1);
  gsl_matrix *y = matrix_check (L, 2);
  const gsl_interp_type *T;
  size_t n = x->size1;
  struct interp *obj;
  bool need_copy;

  if (x->size2 != 1 || y->size2 != 1)
    return luaL_error (L, "both arguments should be column matrix");

  if (x->size1 != y->size1)
    return luaL_error (L, "mismatch in argument's dimensions");

  if (lua_gettop (L) > 2)
    {
      const char *interp_name = luaL_checkstring (L, 3);
      T = interp_algo_lookup (interp_name);
      if (T == NULL)
	return luaL_error (L, "unknown algorithm type");
    }
  else
    {
      T = gsl_interp_linear;
    }

  obj = gs_new_object (sizeof(struct interp), L, GS_INTERP);
  obj->interp = gsl_interp_alloc (T, n);
  obj->acc = gsl_interp_accel_alloc ();
  obj->n = n;

  need_copy = (x->tda != 1 || y->tda != 1);

  if (need_copy)
    {
      gsl_matrix *xsrc = matrix_push_raw (L, n, 1);
      gsl_matrix *ysrc = matrix_push_raw (L, n, 1);
      gsl_matrix_memcpy (xsrc, x);
      gsl_matrix_memcpy (ysrc, y);
      obj->xsrc = xsrc->data;
      obj->ysrc = ysrc->data;
    }
  else
    {
      obj->xsrc = x->data;
      obj->ysrc = y->data;
    }
  
  gsl_interp_init (obj->interp, obj->xsrc, obj->ysrc, n);

  if (need_copy)
    lua_pop (L, 2);

  interp_set_ref (L, 2, 3);

  return 1;
};

int
interp_free (lua_State *L)
{
  struct interp *obj = gs_check_userdata (L, 1, GS_INTERP);
  gsl_interp_free (obj->interp);
  gsl_interp_accel_free (obj->acc);
  return 0;
}

int
interp_eval (lua_State *L)
{
  struct interp *obj = gs_check_userdata (L, 1, GS_INTERP);
  double x = gs_check_number (L, 2, true);
  double y = gsl_interp_eval (obj->interp, obj->xsrc, obj->ysrc, x, obj->acc);
  lua_pushnumber (L, y);
  return 1;
};

int
interp_deriv (lua_State *L)
{
  struct interp *obj = gs_check_userdata (L, 1, GS_INTERP);
  double x = gs_check_number (L, 2, true);
  double d = gsl_interp_eval_deriv (obj->interp, obj->xsrc, obj->ysrc, x, obj->acc);
  lua_pushnumber (L, d);
  return 1;
};

int
interp_deriv2 (lua_State *L)
{
  struct interp *obj = gs_check_userdata (L, 1, GS_INTERP);
  double x = gs_check_number (L, 2, true);
  double d = gsl_interp_eval_deriv2 (obj->interp, obj->xsrc, obj->ysrc, x, obj->acc);
  lua_pushnumber (L, d);
  return 1;
};

int
interp_integ (lua_State *L)
{
  struct interp *obj = gs_check_userdata (L, 1, GS_INTERP);
  double a = gs_check_number (L, 2, true);
  double b = gs_check_number (L, 3, true);
  double v = gsl_interp_eval_integ (obj->interp, obj->xsrc, obj->ysrc, a, b, obj->acc);
  lua_pushnumber (L, v);
  return 1;
};

void
interp_register (lua_State *L)
{
  luaL_newmetatable (L, GS_METATABLE(GS_INTERP));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, interp_methods);
  lua_pop (L, 1);

  luaL_register (L, NULL, interp_functions);
}
