
#include <lua.h>
#include <lauxlib.h>
#include <assert.h>
#include "gs-types.h"

#define GS_MATRIX_NAME_DEF      "GSL.matrix"
#define GS_CMATRIX_NAME_DEF     "GSL.cmatrix"
#define GS_NLINFIT_NAME_DEF     "GSL.solver"
#define GS_CNLINFIT_NAME_DEF    "GSL.csolver"
#define GS_ODESOLV_NAME_DEF     "GSL.ode"
#define GS_CODESOLV_NAME_DEF    "GSL.code"
#define GS_HALFCMPL_R2_NAME_DEF "GSL.ffthcr2"
#define GS_HALFCMPL_MR_NAME_DEF "GSL.ffthcmr"
#define GS_FDFMULTIMIN_NAME_DEF "GSL.fdfmmin"
#define GS_FMULTIMIN_NAME_DEF   "GSL.fmmin"

const struct gs_type gs_type_table[] = {
  {GS_MATRIX,      GS_MATRIX_NAME_DEF,      "real matrix"},
  {GS_CMATRIX,     GS_CMATRIX_NAME_DEF,     "complex matrix"},
  {GS_NLINFIT,     GS_NLINFIT_NAME_DEF,     "real values non-linear solver"},
  {GS_CNLINFIT,    GS_CNLINFIT_NAME_DEF,    "complex values non-linear solver"},
  {GS_ODESOLV,     GS_ODESOLV_NAME_DEF,     "real values ODE solver"},
  {GS_CODESOLV,    GS_CODESOLV_NAME_DEF,    "complex values ODE solver"},
  {GS_HALFCMPL_R2, GS_HALFCMPL_R2_NAME_DEF, "half complex array (radix2)"},
  {GS_HALFCMPL_MR, GS_HALFCMPL_MR_NAME_DEF, "half complex array (mixed radix)"},
  {GS_FDFMULTIMIN, GS_FDFMULTIMIN_NAME_DEF, "fdf multimin solver"}, 
  {GS_FDFMULTIMIN, GS_FMULTIMIN_NAME_DEF,   "f multimin solver"}, 
};

/*
const char * const GS_MATRIX_NAME      = GS_MATRIX_NAME_DEF;
const char * const GS_CMATRIX_NAME     = GS_CMATRIX_NAME_DEF;
const char * const GS_NLINFIT_NAME     = GS_NLINFIT_NAME_DEF;
const char * const GS_CNLINFIT_NAME    = GS_CNLINFIT_NAME_DEF;
const char * const GS_ODESOLV_NAME     = GS_ODESOLV_NAME_DEF;
const char * const GS_CODESOLV_NAME    = GS_CODESOLV_NAME_DEF;
const char * const GS_HALFCMPL_R2_NAME = GS_HALFCMPL_R2_NAME_DEF;
const char * const GS_HALFCMPL_MR_NAME = GS_HALFCMPL_MR_NAME_DEF;
const char * const GS_FDFMULTIMIN_NAME = GS_FDFMULTIMIN_DEF;
*/

const char *
type_qualified_name (int typeid)
{
  const struct gs_type *tp = &gs_type_table[typeid];
  /*  assert (typeid >= 0 && typeid < GS_INVALID_TYPE); */
  return tp->fullname;
}

const char *
metatable_name (int typeid)
{
  const struct gs_type *tp = &gs_type_table[typeid];
  /*  assert (typeid >= 0 && typeid < GS_INVALID_TYPE); */
  return tp->mt_name;
}

static const char *
userdata_full_name (lua_State *L, int index)
{
  if (lua_getmetatable(L, index))
    {
      int j;
      for (j = 0; j < GS_INVALID_TYPE; j++)
	{
	  const char *mt = metatable_name (j);

	  lua_getfield(L, LUA_REGISTRYINDEX, mt);
	  if (lua_rawequal(L, -1, -2)) 
	    {
	      lua_pop (L, 2);
	      return type_qualified_name (j);
	    }
	  lua_pop (L, 1);
	}
    }

  return lua_typename (L, LUA_TUSERDATA);;
}

const char *
full_type_name (lua_State *L, int narg)
{
  int t = lua_type (L, narg);
  const char *nm;

#ifdef LUA_TINT
  assert( t != LUA_TINT );
#endif

  switch (t)
    {
    case LUA_TUSERDATA:
      nm = userdata_full_name (L, narg);
      break;
    default:
      nm = lua_typename (L, t);
    }

  return nm;
}

int
gs_type_error (lua_State *L, int narg, const char *req_type)
{
  const char *actual_type = full_type_name (L, narg);
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
				    req_type, actual_type);
  return luaL_argerror(L, narg, msg);
}

void *
gs_check_userdata (lua_State *L, int index, int typeid)
{
  void *p = lua_touserdata (L, index);

  if (p == NULL)
    gs_type_error (L, index, type_qualified_name (typeid));

  if (lua_getmetatable(L, index))
    {
      const char *mt = metatable_name (typeid);

      lua_getfield(L, LUA_REGISTRYINDEX, mt);
      if (lua_rawequal(L, -1, -2)) 
	{
	  lua_pop (L, 2);
	  return p;
	}
    }

  gs_type_error (L, index, type_qualified_name (typeid));
  return NULL;
}
