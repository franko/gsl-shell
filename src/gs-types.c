
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include "lua-defs.h"
#include "gs-types.h"
#include <math.h>

#define GS_WINDOW_NAME_DEF  "GSL.window"
#define GS_DRAW_SCALABLE_NAME_DEF NULL
#define GS_DRAW_PATH_NAME_DEF   "GSL.path"
#define GS_DRAW_ELLIPSE_NAME_DEF   "GSL.ellipse"
#define GS_DRAW_DRAWABLE_NAME_DEF NULL
#define GS_DRAW_TEXT_NAME_DEF   "GSL.text"
#define GS_DRAW_TEXTSHAPE_NAME_DEF "GSL.textshape"
#define GS_DRAW_MARKER_NAME_DEF "GSL.marker"
#define GS_PLOT_NAME_DEF  "GSL.plot"

#define MYCAT2x(a,b) a ## _ ## b
#define MYCAT2(a,b) MYCAT2x(a,b)
#define MYCAT3x(a,b,c) a ## _ ## b ## _ ## c
#define MYCAT3(a,b,c) MYCAT3x(a,b,c)

#define MY_EXPAND(NM,DESCR) {MYCAT2(GS,NM), MYCAT3(GS,NM,NAME_DEF), DESCR, GS_NO_TYPE}
#define MY_EXPAND_DER(NM,DESCR,BASE) {MYCAT2(GS,NM), MYCAT3(GS,NM,NAME_DEF), DESCR, MYCAT2(GS,BASE)}

const struct gs_type gs_type_table[] = {
  MY_EXPAND(WINDOW, "graphical window"),
  MY_EXPAND(DRAW_SCALABLE, "graphical object"),
  MY_EXPAND_DER(DRAW_PATH, "geometric line", DRAW_SCALABLE),
  MY_EXPAND_DER(DRAW_ELLIPSE, "geometric ellipse", DRAW_SCALABLE),
  MY_EXPAND(DRAW_DRAWABLE, "window graphical object"),
  MY_EXPAND_DER(DRAW_TEXT, "graphical text", DRAW_DRAWABLE),
  MY_EXPAND_DER(DRAW_TEXTSHAPE, "geometric text shape", DRAW_DRAWABLE),
  MY_EXPAND_DER(DRAW_MARKER, "marker point", DRAW_DRAWABLE),
  MY_EXPAND(PLOT, "plot"),
  {GS_INVALID_TYPE, NULL, NULL, GS_NO_TYPE}
};

#undef MYCAT2
#undef MYCAT2x
#undef MYCAT3
#undef MYCAT3x
#undef MY_EXPAND
#undef MY_EXPAND_DER

const char *
type_qualified_name (int typeid)
{
  const struct gs_type *tp = &gs_type_table[typeid];
  return tp->fullname;
}

const char *
metatable_name (int typeid)
{
  const struct gs_type *tp = &gs_type_table[typeid];
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

	  if (mt == NULL)
	    continue;

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
gs_new_object (size_t nbytes, lua_State *L, enum gs_type_e tp)
{
  void* p = lua_newuserdata(L, nbytes);
  gs_set_metatable (L, tp);
  return p;
}

static int
rec_check_type (lua_State *L, enum gs_type_e tp)
{
  const char *mt = metatable_name (tp);
  const struct gs_type *t;

  if (mt)
    {
      lua_getfield(L, LUA_REGISTRYINDEX, mt);
      if (lua_rawequal(L, -1, -2))
	{
	  lua_pop (L, 1);
	  return 1;
	}
      lua_pop (L, 1);
    }

  /* we start to search from tp because we assume that derived type are
     follows base type in the table. */
  for (t = &gs_type_table[tp]; t->tp != GS_INVALID_TYPE; t++)
    {
      if (t->base_type == tp)
	{
	  if (rec_check_type (L, t->tp))
	    return 1;
	}
    }

  return 0;
}

void *
gs_is_userdata (lua_State *L, int index, int typeid)
{
  INDEX_SET_ABS(L, index);

  void *p = lua_touserdata (L, index);

  if (p == NULL)
    return NULL;

  if (lua_getmetatable(L, index))
    {
      if (rec_check_type (L, typeid))
	{
	  lua_pop (L, 1);
	  return p;
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
  if (!lua_isnumber (L, index))
    return gs_type_error (L, index, "real number");
  v = lua_tonumber (L, index);
  if (check_normal)
    {
      if (isinf(v) || isnan(v))
	return luaL_error (L, "invalid 'nan' or 'inf' number", index);
    }
  return v;
}
