
#include <pthread.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "agg_color_rgba.h"

#include "lua-plot.h"
#include "lua-draw.h"
#include "lua-cpp-utils.h"
#include "lua-utils.h"
#include "gs-types.h"
#include "vertex-source.h"
#include "trans.h"
#include "colors.h"
#include "xwin-show.h"

__BEGIN_DECLS

static int agg_plot_new        (lua_State *L);
static int agg_plot_show       (lua_State *L);
static int agg_plot_add        (lua_State *L);
static int agg_plot_update     (lua_State *L);
static int agg_plot_add_line   (lua_State *L);
static int agg_plot_index      (lua_State *L);
static int agg_plot_newindex   (lua_State *L);
static int agg_plot_free       (lua_State *L);
static int agg_plot_title_set  (lua_State *L);
static int agg_plot_title_get  (lua_State *L);
static int agg_plot_units_set  (lua_State *L);
static int agg_plot_units_get  (lua_State *L);

static vertex_source * build_stroke    (lua_State *L, int index, vertex_source *obj);
static vertex_source * build_dash      (lua_State *L, int index, vertex_source *obj);
static vertex_source * build_curve     (lua_State *L, int index, vertex_source *obj);
static vertex_source * build_marker    (lua_State *L, int index, vertex_source *obj);
static vertex_source * build_rotate    (lua_State *L, int index, vertex_source *obj);
static vertex_source * build_translate (lua_State *L, int index, vertex_source *obj);

static const struct luaL_Reg plot_functions[] = {
  {"path",     agg_path_new},
  {"text",     agg_text_new},
  {"rgba",     agg_rgba_new},
  {"rgb",      agg_rgb_new},
  {"plot",     agg_plot_new},
  {NULL, NULL}
};

static const struct luaL_Reg agg_plot_methods[] = {
  {"show",        agg_plot_show       },
  {"add",         agg_plot_add        },
  {"addline",     agg_plot_add_line   },
  {"update",      agg_plot_update     },
  {"__index",     agg_plot_index      },
  {"__newindex",  agg_plot_newindex   },
  {"__gc",        agg_plot_free       },
  {NULL, NULL}
};

static const struct luaL_Reg agg_plot_properties_get[] = {
  {"title",        agg_plot_title_get  },
  {"units",        agg_plot_units_get  },
  {NULL, NULL}
};

static const struct luaL_Reg agg_plot_properties_set[] = {
  {"title",        agg_plot_title_set  },
  {"units",        agg_plot_units_set  },
  {NULL, NULL}
};

struct property_reg {
  int id;
  const char *name;
};

struct builder_reg {
  const char *name;
  vertex_source *(*func)(lua_State *, int, vertex_source *);
};

__END_DECLS
  

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

void agg_plot::wait_update()
{
  if (this->window)
    update_callback (this->window);
};

agg_plot* agg_plot::arg_check(lua_State *L, int index)
{
  return (agg_plot *) gs_check_userdata (L, index, GS_DRAW_PLOT);
}

int
agg_plot_new (lua_State *L)
{
  agg_plot *p = new(L, GS_DRAW_PLOT) agg_plot();

  lua_newtable (L);
  lua_setfenv (L, -2);

  if (lua_isstring (L, 1))
    {
      const char *title = lua_tostring (L, 1);
      if (title)
	p->set_title(title);
    }

  return 1;
}

int
agg_plot_free (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  p->~agg_plot();
  return 0;
}

int
agg_plot_index (lua_State *L)
{
  return mlua_index_with_properties (L, agg_plot_properties_get, false);
}

int
agg_plot_title_set (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  const char *title = lua_tostring (L, 2);

  if (title == NULL)
    return gs_type_error (L, 2, "string");
	  
  AGG_LOCK();

  p->set_title(title);
  p->wait_update();

  AGG_UNLOCK();
	  
  return 0;
}

int
agg_plot_title_get (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  const char *title = p->get_title();
  lua_pushstring (L, title);
  return 1;
}

int
agg_plot_units_set (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  bool request = (bool) lua_toboolean (L, 2);
  bool current = p->use_units();

  if (current != request)
    {
      AGG_LOCK();
      p->set_units(request);
      p->wait_update();
      AGG_UNLOCK();
    }
	  
  return 0;
}

int
agg_plot_units_get (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  lua_pushboolean (L, p->use_units());
  return 1;
}

int
agg_plot_newindex (lua_State *L)
{
  return mlua_newindex_with_properties (L, agg_plot_properties_set);
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
lparse_spec_pipeline (lua_State *L, int index, vertex_source *obj)
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

static agg::rgba8 *
color_arg_lookup (lua_State *L, int index)
{
  agg::rgba8 *c;

  if (lua_isnil (L, index))
    {
      c = rgba8_push_default (L);
      lua_replace (L, index);
    }
  else if (lua_isstring (L, index))
    {
      const char *cstr = lua_tostring (L, index);
      c = rgba8_push_lookup (L, cstr);
      lua_replace (L, index);
    }
  else
    {
      c = check_agg_rgba8 (L, index);
    }

  return c;
}

static int
agg_plot_add_gener (lua_State *L, bool as_line)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  int narg = lua_gettop (L);
  agg::rgba8 *color;

  if (narg <= 2)
    color = rgba8_push_default (L);
  else
    color = color_arg_lookup (L, 3);
      
  if (narg > 5)
    return luaL_error (L, "too much arguments if add or addline plot method");

  vertex_source *curr = check_agg_obj (L, 2);

  if (narg > 4)
    {
      curr = lparse_spec_pipeline (L, 5, curr);
      lua_pop (L, 1);
    }
    
  if (curr->need_resize())
    {
      curr = new trans::resize(curr);
    }

  if (narg > 3)
    {
      curr = lparse_spec_pipeline (L, 4, curr);
      lua_pop (L, 1);
    }

  lua_pushvalue (L, 1);
  mlua_fenv_addref (L, 2);
  lua_pop (L, 1);

  AGG_LOCK();
  p->add(curr, color, as_line);
  p->wait_update();
  AGG_UNLOCK();

  return 0;
}
 
int
agg_plot_add (lua_State *L)
{
  return agg_plot_add_gener (L, false);
}
 
int
agg_plot_add_line (lua_State *L)
{
  return agg_plot_add_gener (L, true);
}
 
int
agg_plot_update (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  AGG_PROTECT(p->wait_update());
  return 0;
}

int
agg_plot_show (lua_State *L)
{
  agg_plot *p = agg_plot::arg_check(L, 1);
  pthread_t xwin_thread[1];
  pthread_attr_t attr[1];

  AGG_LOCK();

  if (! p->is_shown)
    {
      p->id = mlua_window_ref(L, 1);

      pthread_attr_init (attr);
      pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

      if (pthread_create(xwin_thread, attr, xwin_thread_function, (void*) p))
	{
	  pthread_attr_destroy (attr);
	  mlua_window_unref(L, p->id);
	  p->id = -1;
	  AGG_UNLOCK();
	  return luaL_error(L, "error creating thread.");
	}
      
      p->is_shown = 1;
      pthread_attr_destroy (attr);
    }

  AGG_UNLOCK();

  return 0;
}

void
plot_register (lua_State *L)
{
  /* plot declaration */
  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_PLOT));
  luaL_register (L, NULL, agg_plot_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, plot_functions);
}
