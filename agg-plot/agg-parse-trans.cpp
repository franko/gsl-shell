
#include <string.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <memory>

#include "agg-parse-trans.h"
#include "lua-defs.h"
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

const char *agg_spec_error::m_msg[] = {
  "invalid specification tag",
  "invalid specification table",
  "missing specification parameter",
  "invalid graphical object",
  "generic error during graphical operation"
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

	  double b = (lua_isnumber (L, -1) ? lua_tonumber (L, -1) : a);

	  dash.add_dash(a, b);
	  lua_pop (L,1);
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

template <class context> typename context::base_type *
affine_object_compose(typename context::base_type *obj, agg::trans_affine& m)
{
  typedef typename trans<context>::affine affine_type;

  if (obj->affine_compose(m))
    {
      return obj;
    }

  return (typename context::base_type *) new affine_type(obj, m);
}

template <class context> typename context::base_type*
build_translate (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef typename trans<context>::affine affine_type;

  double x = mlua_named_number (L, specindex, "x");
  double y = mlua_named_number (L, specindex, "y");

  agg::trans_affine mtx(1.0, 0.0, 0.0, 1.0, x, y);
  return affine_object_compose<context>(obj, mtx);
}

template <class context> typename context::base_type*
build_scale (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef typename trans<context>::affine affine_type;

  lua_rawgeti (L, specindex, 2);

  if (! lua_isnumber (L, -1))
    throw agg_spec_error(agg_spec_error::missing_parameter);

  double s = lua_tonumber (L, -1);
  lua_pop (L, 1);

  agg::trans_affine mtx(s, 0.0, 0.0, s, 0.0, 0.0);
  return affine_object_compose<context>(obj, mtx);
}

template <class context> typename context::base_type*
build_rotate (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef typename trans<context>::affine affine_type;

  double a = mlua_named_number (L, specindex, "angle");

  double c = cos(a), s = sin(a);
  agg::trans_affine mtx(c, s, -s, c, 0.0, 0.0);
  return affine_object_compose<context>(obj, mtx);
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
  {"scale",         build_scale    <context>},
  {"rotate",        build_rotate   <context>},
  {NULL, NULL}
};

template <class context> typename context::base_type *
parse_spec (lua_State *L, int specindex, typename context::base_type *obj)
{
  typedef builder<context> builder_type;
  const char *tag;

  INDEX_SET_ABS(L, specindex);

  if (lua_type (L, specindex) != LUA_TTABLE)
    throw agg_spec_error(agg_spec_error::invalid_spec);

  lua_rawgeti (L, specindex, 1);
  if (! lua_isstring (L, -1))
    throw agg_spec_error(agg_spec_error::invalid_tag);

  tag = lua_tostring (L, -1);
  lua_pop (L, 1);

  typename builder_type::func_type *f = builder_type::lookup(tag);

  if (f == 0) throw agg_spec_error(agg_spec_error::invalid_tag);

  typename context::base_type *retval = f (L, specindex, obj);
  return retval;
}

template <class context> typename context::base_type *
parse_spec_pipeline (lua_State *L, int index, typename context::base_type *obj)
{
  if (lua_type (L, index) != LUA_TTABLE)
    throw agg_spec_error(agg_spec_error::invalid_spec);

  size_t k, nb = lua_objlen (L, index);

  for (k = nb; k > 0; k--)
    {
      lua_rawgeti (L, index, k);
      obj = parse_spec<context> (L, -1, obj);
      lua_pop (L, 1);
    }

  return obj;
}

drawable *
parse_graph_args (lua_State *L, agg::rgba8& color)
{
  color = color_arg_lookup (L, 3);

  std::auto_ptr<drawable> wobj;

  if (gs_is_userdata (L, 2, GS_DRAW_SCALABLE))
    {
      vertex_source *vs = (vertex_source *) lua_touserdata (L, 2);
      std::auto_ptr<scalable> sobj(new boxed_scalable(vs));

      if (! lua_isnoneornil (L, 5))
	{
	  scalable *st = parse_spec_pipeline<scalable_context> (L, 5, sobj.get());
	  sobj.release();
	  sobj = std::auto_ptr<scalable>(st);
	}
    
      drawable *ws = new window_scalable(sobj.get());

      sobj.release();
      wobj = std::auto_ptr<drawable>(ws);
    }
  else if (gs_is_userdata (L, 2, GS_DRAW_DRAWABLE))
    {
      drawable *vs = (drawable *) lua_touserdata (L, 2);
      wobj = std::auto_ptr<drawable>(new boxed_drawable(vs));
    }
  else
    {
      throw agg_spec_error(agg_spec_error::invalid_object);
    }

  drawable *w = wobj.get();
  if (! lua_isnoneornil (L, 4))
    {
      w = parse_spec_pipeline<drawable_context> (L, 4, w);
    }

  wobj.release();
  return w;
}
