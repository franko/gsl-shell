
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

sg_object* build_stroke (lua_State *L, int specindex, sg_object* src)
{
  double width = mlua_named_optnumber (L, specindex, "width", 1.0);
  const char *cap_str  = mlua_named_optstring (L, specindex, "cap",  NULL);
  const char *join_str = mlua_named_optstring (L, specindex, "join", NULL);

  trans::stroke* s = new trans::stroke(src);
  trans::conv_stroke& stroke = s->self();

  s->width(width);

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

  return s;
}

sg_object* build_curve (lua_State *L, int specindex, sg_object* src)
{
  return new trans::curve(src);
}

sg_object* build_marker (lua_State *L, int specindex, sg_object* src)
{
  double size = mlua_named_optnumber(L, specindex, "size", 3.0);

  lua_getfield(L, specindex, "mark");
  const char *sym_name = lua_tostring(L, -1);

  sg_object *sym;
  if (!sym_name && gs_is_userdata(L, -1, GS_DRAW_SCALABLE))
    {
      sg_object* obj = (sg_object*) lua_touserdata(L, -1);
      sym = new trans::scaling_a(obj);
    }
  else
    {
      sym = new_marker_symbol(sym_name ? sym_name : "circle");
    }
  lua_pop(L, 1);

  return new trans::marker(src, size, sym);
}

sg_object* build_dash (lua_State *L, int specindex, sg_object* src)
{
  trans::dash* d = new trans::dash(src);

  for (int j = 2; /* */; j += 2)
    {
      lua_rawgeti (L, specindex, j);

      if (lua_isnumber (L, -1))
	{
	  double a = lua_tonumber (L, -1);
	  lua_pop (L, 1);

	  lua_rawgeti (L, specindex, j+1);

	  double b = (lua_isnumber (L, -1) ? lua_tonumber (L, -1) : a);

	  d->add_dash(a, b);
	  lua_pop (L,1);
	}
      else
	break;
    }
  lua_pop (L, 1);

  return d;
}

sg_object* build_extend (lua_State *L, int specindex, sg_object* src)
{
  double width = mlua_named_optnumber (L, specindex, "width", 1.0);
  trans::extend* m = new trans::extend(src);

  agg::conv_contour<sg_object>& e = m->self();
  e.width(width);
  e.auto_detect_orientation(true);

  return m;
}

sg_object* affine_object_compose(sg_object* obj, agg::trans_affine& m)
{
  if (obj->affine_compose(m))
    return obj;

  return new trans::affine(obj, m);
}

sg_object* build_translate (lua_State* L, int specindex, sg_object* src)
{
  double x = mlua_named_number (L, specindex, "x");
  double y = mlua_named_number (L, specindex, "y");

  agg::trans_affine mtx(1.0, 0.0, 0.0, 1.0, x, y);
  return affine_object_compose(src, mtx);
}

sg_object* build_scale (lua_State *L, int specindex, sg_object* src)
{
  lua_rawgeti (L, specindex, 2);

  if (lua_isnumber (L, -1))
    {
      double s = luaL_optnumber (L, -1, 1.0);
      lua_pop (L, 1);

      agg::trans_affine mtx(s, 0.0, 0.0, s, 0.0, 0.0);
      return affine_object_compose(src, mtx);
    }

  return src;
}

sg_object* build_rotate (lua_State *L, int specindex, sg_object* src)
{
  double a = mlua_named_number (L, specindex, "angle");
  double c = cos(a), s = sin(a);
  agg::trans_affine mtx(c, s, -s, c, 0.0, 0.0);
  return affine_object_compose(src, mtx);
}

class builder {
public:
  typedef sg_object* (func_type)(lua_State*, int, sg_object*);

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

const builder::reg builder::builder_table[] = {
  {"stroke",        build_stroke   },
  {"dash",          build_dash     },
  {"curve",         build_curve    },
  {"marker",        build_marker   },
  {"extend",        build_extend   },
  {"translate",     build_translate},
  {"scale",         build_scale    },
  {"rotate",        build_rotate   },
  {NULL, NULL}
};

sg_object* parse_spec (lua_State *L, int specindex, sg_object* src,
                       gslshell::ret_status& st)
{
  const char *tag;

  INDEX_SET_ABS(L, specindex);

  if (lua_type (L, specindex) != LUA_TTABLE)
    {
      st.error("invalid specification table", "plot add");
      return 0;
    }

  lua_rawgeti (L, specindex, 1);
  tag = lua_tostring (L, -1);
  lua_pop (L, 1);

  if (!tag)
    {
      st.error("invalid specification tag", "plot add");
      return 0;
    }

  builder::func_type* f = builder::lookup(tag);

  if (f == 0)
    {
      st.error("invalid specification tag", "plot add");
      return 0;
    }

  return f(L, specindex, src);
}

sg_object* parse_spec_pipeline (lua_State* L, int index, sg_object* src,
                                gslshell::ret_status& st)
{
  if (lua_type (L, index) != LUA_TTABLE)
    {
      st.error("invalid specification table", "plot add");
      return 0;
    }

  size_t k, nb = lua_objlen (L, index);

  sg_object* obj = src;
  for (k = nb; k > 0 && src; k--, src = obj)
    {
      lua_rawgeti (L, index, k);
      obj = parse_spec(L, -1, src, st);
      if (obj == 0)
        delete src;
      lua_pop (L, 1);
    }

  return obj;
}

sg_object* parse_graph_args (lua_State *L, agg::rgba8& color,
                             gslshell::ret_status& st)
{
  color = color_arg_lookup (L, 3);

  sg_object* wobj;

  if (gs_is_userdata (L, 2, GS_DRAW_SCALABLE))
    {
      sg_object* vs = (sg_object*) lua_touserdata (L, 2);
      sg_object* sobj = new sg_object_ref<manage_not_owner>(vs);

      if (! lua_isnoneornil (L, 5))
        {
          sobj = parse_spec_pipeline(L, 5, sobj, st);
          if (!sobj)
            return 0;
        }

      wobj = new trans::scaling(sobj);
    }
  else if (gs_is_userdata (L, 2, GS_DRAW_DRAWABLE))
    {
      sg_object* vs = (sg_object*) lua_touserdata (L, 2);
      wobj = new sg_object_ref<manage_not_owner>(vs);
    }
  else
    {
      st.error("invalid graphical object", "plot add");
      return 0;
    }

  if (! lua_isnoneornil (L, 4))
    {
      wobj = parse_spec_pipeline(L, 4, wobj, st);
    }

  return wobj;
}
