#include <lua.h>
#include <lauxlib.h>
#include <assert.h>
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

struct interp_type_reg {
  const char *name;
  const gsl_interp_type *type;
};

enum fenv_pos {
  FENV_X_VECTOR = 1,
  FENV_Y_VECTOR = 2,
};

static int          interp_new       (lua_State *L);
static int          interp_free      (lua_State *L);
static int          interp_eval      (lua_State *L);

struct interp_type_reg interp_type_table[] = {
#if 0
  {"linear",             gsl_interp_linear},
  {"polynomial",         gsl_interp_polynomial},
  {"cspline",            gsl_interp_cspline},
  {"cspline_periodic",   gsl_interp_cspline_periodic},
  {"akima",              gsl_interp_akima},
  {"akima_periodic",     gsl_interp_akima_periodic},
#endif
  {"linear",             NULL},
  {"polynomial",         NULL},
  {"cspline",            NULL},
  {"cspline_periodic",   NULL},
  {"akima",              NULL},
  {"akima_periodic",     NULL},
  {NULL,                 NULL}
};

static const struct luaL_Reg interp_methods[] = {
  {"__gc",          interp_free},
  {"eval",          interp_eval},
  /*  {"evalvec",       interp_vector_eval}, */
  {NULL, NULL}
};

static const struct luaL_Reg interp_functions[] = {
  {"interp",         interp_new},
  {NULL, NULL}
};  

static struct interp_type_reg *
interp_type_lookup (const char *req_name)
{
  struct interp_type_reg *p;
  for (p = interp_type_table; p->name != NULL; p++)
    {
      if (strcmp (p->name, req_name) == 0)
	return p;
    }

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
  const char *interp_name = luaL_checkstring (L, 1);
  gsl_matrix *x = matrix_check (L, 2);
  gsl_matrix *y = matrix_check (L, 3);
  struct interp_type_reg *reg;
  size_t n = x->size1;
  struct interp *obj;

  if (x->size2 != 1 || y->size2 != 1)
    return luaL_error (L, "both arguments should be column matrix");

  if (x->size1 != y->size1)
    return luaL_error (L, "mismatch in argument's dimensions");

  reg = interp_type_lookup (interp_name);
  if (reg == NULL)
    return luaL_error (L, "the requested type of interpolation is not "
		       "recognized");

  obj = gs_new_object (sizeof(struct interp), L, GS_INTERP);
  obj->interp = gsl_interp_alloc (reg->type, n);
  obj->acc = gsl_interp_accel_alloc ();
  obj->n = n;

  obj->xsrc = x->data;
  obj->ysrc = y->data;
  
  gsl_interp_init (obj->interp, x->data, y->data, n);

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

static void
init_interp_type_table ()
{
  struct interp_type_reg *p = interp_type_table;
  (p++)->type = gsl_interp_linear;
  (p++)->type = gsl_interp_polynomial;
  (p++)->type = gsl_interp_cspline;
  (p++)->type = gsl_interp_cspline_periodic;
  (p++)->type = gsl_interp_akima;
  (p++)->type = gsl_interp_akima_periodic;
}

void
interp_register (lua_State *L)
{
  init_interp_type_table ();

  luaL_newmetatable (L, GS_METATABLE(GS_INTERP));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, interp_methods);
  lua_pop (L, 1);

  luaL_register (L, NULL, interp_functions);
}
