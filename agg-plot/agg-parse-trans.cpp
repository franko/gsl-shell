
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "agg-parse-trans.h"
#include "lua-cpp-utils.h"
#include "lua-utils.h"
#include "gs-types.h"
#include "trans.h"

struct property_reg {
  int id;
  const char *name;
};

struct builder_reg {
  const char *name;
  vertex_source *(*func)(lua_State *, int, vertex_source *);
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

const builder_reg builder_table[] = {
  {"stroke",        build_stroke  },
  {"dash",          build_dash},
  {"curve",         build_curve},
  {"marker",        build_marker},
  {"translate",     build_translate},
  {"rotate",        build_rotate},
  {NULL, NULL}
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

vertex_source *
build_stroke (lua_State *L, int specindex, vertex_source *obj)
{
  double width      = mlua_named_optnumber (L, specindex, "width", 1.0);
  const char *cap_str  = mlua_named_optstring (L, specindex, "cap",  NULL);
  const char *join_str = mlua_named_optstring (L, specindex, "join", NULL);

  trans::stroke *stroke = new trans::stroke(obj, width);

  if (cap_str)
    {
      int cap = property_lookup (line_cap_properties, cap_str);
      stroke->line_cap((agg::line_cap_e) cap);
    }

  if (join_str)
    {
      int join = property_lookup (line_join_properties, join_str);
      stroke->line_join((agg::line_join_e) join);
    }

  return (vertex_source *) stroke;
}

vertex_source *
build_curve (lua_State *L, int specindex, vertex_source *obj)
{
  trans::curve *c = new trans::curve(obj);
  return (vertex_source *) c;
}

vertex_source *
build_marker (lua_State *L, int specindex, vertex_source *obj)
{
  double size = mlua_named_optnumber (L, specindex, "size", 3.0);
  return (vertex_source *) new trans::marker(obj, size);
}

vertex_source *
build_dash (lua_State *L, int specindex, vertex_source *obj)
{
  double a = mlua_named_optnumber (L, specindex, "a", 10.0);
  double b = mlua_named_optnumber (L, specindex, "b", a);

  trans::dash *dash = new trans::dash(obj);
  dash->add_dash(a, b);

  return (vertex_source *) dash;
}

vertex_source *
build_translate (lua_State *L, int specindex, vertex_source *obj)
{
  double x = mlua_named_number (L, specindex, "x");
  double y = mlua_named_number (L, specindex, "y");

  trans::affine *t = new trans::affine(obj);
  t->translate(x, y);

  return (vertex_source *) t;
}

vertex_source *
build_rotate (lua_State *L, int specindex, vertex_source *obj)
{
  double a = mlua_named_number (L, specindex, "angle");

  trans::affine *t = new trans::affine(obj);
  t->rotate(a);

  return (vertex_source *) t;
}

vertex_source *
parse_spec (lua_State *L, int specindex, vertex_source *obj)
{
  const char *tag;

  lua_rawgeti (L, specindex, 1);
  if (! lua_isstring (L, -1))
    {
      luaL_error (L, "the tag of the transformation is invalid");
      return NULL;
    }

  tag = lua_tostring (L, -1);
  lua_pop (L, 1);

  for (const builder_reg *b = builder_table; b->name != NULL; b++)
    {
      if (strcmp (b->name, tag) == 0)
	return b->func (L, specindex, obj);
    }

  luaL_error (L, "invalid trasformation tag");
  return NULL;
}

vertex_source *
parse_spec_pipeline (lua_State *L, int index, vertex_source *obj)
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
      obj = parse_spec (L, index+1, obj);
      lua_pop (L, 1);
    }

  return obj;
}
