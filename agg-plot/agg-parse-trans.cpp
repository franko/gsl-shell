
#include <string.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "agg-parse-trans.h"
#include "lua-cpp-utils.h"
#include "lua-utils.h"
#include "lua-draw.h"
#include "gs-types.h"
#include "colors.h"

#include "scalable.h"
#include "drawable.h"
#include "path.h"
#include "text.h"
#include "trans.h"

struct property_reg {
  int id;
  const char *name;
};

struct property_reg line_cap_properties[] = {
  {(int) agg::butt_cap,   "butt"  },
  {(int) agg::square_cap, "square"},
  {(int) agg::round_cap,  "round" },
  {0, NULL}
};

struct property_reg line_join_properties[] = {
  {(int) agg::miter_join,        "miter"      },
  {(int) agg::miter_join_revert, "miter.rev"  },
  {(int) agg::round_join,        "round"      },
  {(int) agg::bevel_join,        "bevel"      },
  {(int) agg::miter_join_round,  "miter.round"},
  {0, NULL}
};

static int
property_lookup (struct property_reg *prop, const char *key)
{
  int default_value = prop->id;

  if (key == NULL)
    return default_value;

  for ( ; prop->name; prop++)
    {
      if (strcmp (prop->name, key) == 0)
        return prop->id;
    }

  return default_value;
}

template <class context>
typename context::base_type* build_stroke (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef typename trans<context>::stroke stroke_type;

  double width = mlua_named_optnumber (L, specindex, "width", 1.0);
  const char *cap_str  = mlua_named_optstring (L, specindex, "cap",  NULL);
  const char *join_str = mlua_named_optstring (L, specindex, "join", NULL);

  stroke_type *s = new stroke_type(obj);

  typename trans<context>::stroke_base& stroke = s->self();

  stroke.width(width);

  if (cap_str)
    {
      int cap = property_lookup (line_cap_properties, cap_str);
      stroke.line_cap((agg::line_cap_e) cap);
    }

  if (join_str)
    {
      int join = property_lookup (line_join_properties, join_str);
      stroke.line_join((agg::line_join_e) join);
    }

  return (typename context::base_type *) s;
}

template <class context> typename context::base_type*
build_curve (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef typename trans<context>::curve curve_type;
  return (typename context::base_type *) new curve_type(obj);
}

template <class context> typename context::base_type*
build_marker (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef typename trans<context>::marker marker_type;

  double size = mlua_named_optnumber (L, specindex, "size", 3.0);
  const char *mark = mlua_named_optstring (L, specindex, "mark", "circle");
  marker_type *m = new marker_type(obj, size, mark);
  return (typename context::base_type *) m;
}

template <class context> typename context::base_type *
build_dash (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef typename trans<context>::dash dash_type;

  dash_type *d = new dash_type(obj);
  typename trans<context>::dash_base& dash = d->self();

  for (int j = 2; /* */; j += 2)
    {
      lua_rawgeti (L, specindex, j);

      if (lua_isnumber (L, -1))
	{
	  double a = lua_tonumber (L, -1);
	  lua_pop (L, 1);

	  lua_rawgeti (L, specindex, j+1);
	  if (lua_isnumber (L, -1))
	    {
	      double b = lua_tonumber (L, -1);
	      dash.add_dash(a, b);
	      lua_pop (L,1);
	    }
	  else
	    break;
	}
      else
	break;
    }
  lua_pop (L, 1);

  return (typename context::base_type *) d;
}

template <class context> typename context::base_type*
build_extend (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef typename trans<context>::extend extend_type;

  double width = mlua_named_optnumber (L, specindex, "width", 1.0);
  extend_type *m = new extend_type(obj);

  typename trans<context>::extend_base& e = m->self();
  e.width(width);
  e.auto_detect_orientation(true);

  return (typename context::base_type *) m;
}

template <class context> typename context::base_type*
build_translate (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef typename trans<context>::affine affine_type;

  double x = mlua_named_number (L, specindex, "x");
  double y = mlua_named_number (L, specindex, "y");

  agg::trans_affine mtx(1.0, 0.0, 0.0, 1.0, x, y);
  affine_type *t = new affine_type(obj, mtx);

  return (typename context::base_type *) t;
}

template <class context> typename context::base_type*
build_rotate (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef typename trans<context>::affine affine_type;

  double a = mlua_named_number (L, specindex, "angle");

  double c = cos(a), s = sin(a);
  agg::trans_affine mtx(c, s, -s, c, 0.0, 0.0);
  affine_type *t = new affine_type(obj, mtx);

  return (typename context::base_type *) t;
}

template <class context>
class builder {
  typedef typename context::base_type base_type;
public:
  typedef base_type *(func_type)(lua_State *, int, base_type *);

  struct reg {
    const char *name;
    func_type *func;
  };

private:

  static const reg builder_table[];

public:
  static func_type* lookup(const char *key)
  {
    const reg *p;
    for (p = builder_table; p->name != NULL; p++)
      {
	if (strcmp (p->name, key) == 0)
	  return p->func;
      }

    return NULL;
  }
};

template <class context>
const typename builder<context>::reg builder<context>::builder_table[] = {
  {"stroke",        build_stroke   <context>},
  {"dash",          build_dash     <context>},
  {"curve",         build_curve    <context>},
  {"marker",        build_marker   <context>},
  {"extend",        build_extend   <context>},
  {"translate",     build_translate<context>},
  {"rotate",        build_rotate   <context>},
  {NULL, NULL}
};

template <class context> typename context::base_type *
parse_spec (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef builder<context> builder_type;

  const char *tag;

  lua_rawgeti (L, specindex, 1);
  if (! lua_isstring (L, -1))
    {
      luaL_error (L, "the tag of the transformation is invalid");
      return NULL;
    }

  tag = lua_tostring (L, -1);
  lua_pop (L, 1);

  typename builder_type::func_type *f = builder_type::lookup(tag);

  if (f)
    return f (L, specindex, obj);

  luaL_error (L, "invalid trasformation tag");
  return NULL;
}

template <class context> typename context::base_type *
parse_spec_pipeline (lua_State *L, int index, typename context::base_type *obj)
{
  size_t k, nb;

  if (lua_type (L, index) == LUA_TTABLE)
    nb = lua_objlen (L, index);
  else
    {
      luaL_error (L, "post transform argument should be a table");
      return NULL;
    }

  for (k = nb; k > 0; k--)
    {
      lua_rawgeti (L, index, k);
      obj = parse_spec<context> (L, index+1, obj);
      lua_pop (L, 1);
    }

  return obj;
}

drawable *
parse_graph_args (lua_State *L)
{
  int narg = lua_gettop (L);
  agg::rgba8 *color;

  if (narg <= 2)
    color = rgba8_push_default (L);
  else
    color = color_arg_lookup (L, 3);
      
  if (narg > 5)
    {
      luaL_error (L, "too much arguments if add or addline plot method");
      return NULL;
    }

  scalable *s = check_agg_scalable (L, 2);

  if (narg > 4)
    {
      s = parse_spec_pipeline<scalable_context> (L, 5, s);
      lua_pop (L, 1);
    }
    
  drawable *w = new window_scalable(s);

  if (narg > 3)
    {
      w = parse_spec_pipeline<drawable_context> (L, 4, w);
      lua_pop (L, 1);
    }

  return w;
}
