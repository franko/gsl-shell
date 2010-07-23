
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include "gs-types.h"
#include <gsl/gsl_errno.h>
#include <math.h>

static int gs_type_string (lua_State *L);

#define GS_MATRIX_NAME_DEF      "GSL.matrix"
#define GS_CMATRIX_NAME_DEF     "GSL.cmatrix"
#define GS_RNG_NAME_DEF         "GSL.rng"
#define GS_NLINFIT_NAME_DEF     "GSL.solver"
#define GS_CNLINFIT_NAME_DEF    "GSL.csolver"
#define GS_ODESOLV_NAME_DEF     "GSL.ode"
#define GS_CODESOLV_NAME_DEF    "GSL.code"
#define GS_HALFCMPL_R2_NAME_DEF "GSL.ffthcr2"
#define GS_HALFCMPL_MR_NAME_DEF "GSL.ffthcmr"
#define GS_FDFMULTIMIN_NAME_DEF "GSL.fdfmmin"
#define GS_FMULTIMIN_NAME_DEF   "GSL.fmmin"
#define GS_BSPLINE_NAME_DEF     "GSL.bspline"
#ifdef AGG_PLOT_ENABLED
#define GS_DRAW_PLOT_NAME_DEF   "GSL.plot"
#define GS_DRAW_PATH_NAME_DEF   "GSL.path"
#define GS_DRAW_TEXT_NAME_DEF   "GSL.text"
#define GS_RGBA_COLOR_NAME_DEF  "GSL.rgba"
#define GS_CANVAS_WINDOW_NAME_DEF  "GSL.canvas"
#define GS_PLOT_WINDOW_NAME_DEF  "GSL.pltcanvas"
#endif

#define MYCAT2x(a,b) a ## _ ## b
#define MYCAT2(a,b) MYCAT2x(a,b)
#define MYCAT3x(a,b,c) a ## _ ## b ## _ ## c
#define MYCAT3(a,b,c) MYCAT3x(a,b,c)

#define MY_EXPAND(NM,DESCR) {MYCAT2(GS,NM), MYCAT3(GS,NM,NAME_DEF), DESCR, NULL}
#define MY_EXPAND_DER(NM,DESCR,SUB) {MYCAT2(GS,NM), MYCAT3(GS,NM,NAME_DEF), DESCR, &gs_type_table[MYCAT2(GS,SUB)]}

const struct gs_type gs_type_table[] = {
  MY_EXPAND(MATRIX, "real matrix"),
  MY_EXPAND(CMATRIX, "complex matrix"),
  MY_EXPAND(RNG, "random number generator"),
  MY_EXPAND(NLINFIT, "real values non-linear solver"),
  MY_EXPAND(CNLINFIT, "complex values non-linear solver"),
  MY_EXPAND(ODESOLV, "real values ODE solver"),
  MY_EXPAND(CODESOLV, "complex values ODE solver"),
  MY_EXPAND(HALFCMPL_R2, "half complex array (radix2)"),
  MY_EXPAND(HALFCMPL_MR, "half complex array (mixed radix)"),
  MY_EXPAND(FDFMULTIMIN, "fdf multimin solver"), 
  MY_EXPAND(FMULTIMIN, "f multimin solver"), 
  MY_EXPAND(BSPLINE, "B-spline"), 
#ifdef AGG_PLOT_ENABLED
  MY_EXPAND(DRAW_PLOT, "plot"),
  MY_EXPAND(DRAW_PATH, "geometric line"),
  MY_EXPAND(DRAW_TEXT, "graphical text"),
  MY_EXPAND(RGBA_COLOR, "color"),
  MY_EXPAND(PLOT_WINDOW, "plot window"),
  MY_EXPAND_DER(CANVAS_WINDOW, "graphical window", PLOT_WINDOW),
#endif
};

#undef MYCAT2
#undef MYCAT2x
#undef MYCAT3
#undef MYCAT3x
#undef MY_EXPAND
#undef MY_EXPAND_DER

const struct luaL_Reg gs_type_functions[] = {
  {"gsltype",        gs_type_string},
  {NULL, NULL}
};  

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
gs_is_userdata (lua_State *L, int index, int typeid)
{
  void *p = lua_touserdata (L, index);

  if (p == NULL)
    return NULL;

  if (lua_getmetatable(L, index))
    {
      const struct gs_type *inf;

      for (inf = &gs_type_table[typeid]; inf != NULL; inf = inf->derived_class)
	{
	  const char *mt = metatable_name (inf->tp);

	  lua_getfield(L, LUA_REGISTRYINDEX, mt);
	  if (lua_rawequal(L, -1, -2)) 
	    {
	      lua_pop (L, 2);
	      return p;
	    }

	  lua_pop (L, 1);
	}
    }

  lua_pop (L, 1);

  return NULL;
}

void *
gs_check_userdata (lua_State *L, int index, int typeid)
{
  void *p = gs_is_userdata (L, index, typeid);

  if (p == NULL)
    gs_type_error (L, index, type_qualified_name (typeid));

  return p;
}

void *
gs_check_userdata_w_alt (lua_State *L, int index, int typeid1, int typeid2,
			 int *sel)
{
  void *p;

  p = gs_is_userdata (L, index, typeid1);
  if (p == NULL)
    {
      p = gs_is_userdata (L, index, typeid2);
      if (p == NULL)
	{
	  const char *msg = lua_pushfstring(L, "%s or %s", 
					    type_qualified_name (typeid1),
					    type_qualified_name (typeid2));
	  gs_type_error (L, index, msg);
	}

      if (sel)
	*sel = typeid2;
    }

  if (sel)
    *sel = typeid1;

  return p;
}

int
gs_gsl_errorcheck (lua_State *L, const char *routine, int status)
{
  if (status)
    {
      return luaL_error (L, "error during %s: %s", routine,
			 gsl_strerror (status));
    }
  return 0;
}

void
gs_set_metatable (lua_State *L, int typeid)
{
  luaL_getmetatable (L, GS_METATABLE(typeid));
  lua_setmetatable (L, -2);
}

int
gs_type_string (lua_State *L)
{
  const char *name = full_type_name (L, 1);
  lua_pushstring (L, name);
  return 1;
}

double
gs_check_number (lua_State *L, int index, int check_normal)
{
  double v;
  int fpclass;
  if (!lua_isnumber (L, index))
    return gs_type_error (L, index, "real number");
  v = lua_tonumber (L, index);
  if (check_normal)
    {
      fpclass = fpclassify (v);
      if (fpclass == FP_NAN || fpclass == FP_INFINITE)
	return luaL_error (L, "invalid 'nan' or 'inf' number", index);
    }
  return v;
}
