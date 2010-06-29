
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

static int build_stroke        (lua_State *L);
static int build_dash          (lua_State *L);
static int build_curve         (lua_State *L);
static int build_marker        (lua_State *L);
static int build_rotate        (lua_State *L);
static int build_translate     (lua_State *L);

/* DEBUG DEBUG DEBUG */
static int window_debug_list   (lua_State *L);
static int obj_getfenv         (lua_State *L);

static const struct luaL_Reg plot_functions[] = {
  {"path",     agg_path_new},
  {"text",     agg_text_new},
  {"rgba",     agg_rgba_new},
  {"rgb",      agg_rgb_new},
  {"plot",     agg_plot_new},
  {"windows",  window_debug_list},
  {"objgetfenv",  obj_getfenv},
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

const struct luaL_Reg trans_builder[] = {
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
  printf("freeing plot\n");
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

int
build_stroke (lua_State *L)
{
  const int specindex = 3, objindex = 2;
  double width      = mlua_named_optnumber (L, specindex, "width", 1.0);
  const char *scap  = mlua_named_optstring (L, specindex, "cap",  NULL);
  const char *sjoin = mlua_named_optstring (L, specindex, "join", NULL);
  vertex_source *obj = check_agg_obj (L, objindex);

  trans::stroke *stroke = new(L, GS_DRAW_OBJ) trans::stroke(obj, width);
  mlua_set_fenv_ref (L, objindex);

  if (scap)
    {
      int cap = property_lookup (line_cap_properties, scap);
      stroke->line_cap((agg::line_cap_e) cap);
    }

  if (sjoin)
    {
      int join = property_lookup (line_join_properties, sjoin);
      stroke->line_join((agg::line_join_e) join);
    }

  return 1;
}

int
build_curve (lua_State *L)
{
  const int specindex = 3, objindex = 2;
  vertex_source *obj = check_agg_obj (L, objindex);

  trans::curve *c = new(L, GS_DRAW_OBJ) trans::curve(obj);
  mlua_set_fenv_ref (L, objindex);

  return 1;
}

int
build_marker (lua_State *L)
{
  const int specindex = 3, objindex = 2;
  double size = mlua_named_optnumber (L, specindex, "size", 3.0);
  vertex_source *obj = check_agg_obj (L, objindex);

  new(L, GS_DRAW_OBJ) trans::marker(obj, size);
  mlua_set_fenv_ref (L, objindex);

  return 1;
}

int
build_dash (lua_State *L)
{
  const int specindex = 3, objindex = 2;
  double a = mlua_named_optnumber (L, specindex, "a", 10.0);
  double b = mlua_named_optnumber (L, specindex, "b", a);
  vertex_source *obj = check_agg_obj (L, objindex);

  trans::dash *dash = new(L, GS_DRAW_OBJ) trans::dash(obj);
  mlua_set_fenv_ref (L, objindex);
 
  dash->add_dash(a, b);

  return 1;
}

int
build_translate (lua_State *L)
{
  const int specindex = 3, objindex = 2;
  double x = mlua_named_number (L, specindex, "x");
  double y = mlua_named_number (L, specindex, "y");
  vertex_source *obj = check_agg_obj (L, objindex);

  trans::affine *t = new(L, GS_DRAW_OBJ) trans::affine(obj);
  mlua_set_fenv_ref (L, objindex);

  t->translate(x, y);

  return 1;
}

int
build_rotate (lua_State *L)
{
  const int specindex = 3, objindex = 2;
  double a = mlua_named_number (L, specindex, "angle");
  vertex_source *obj = check_agg_obj (L, objindex);

  trans::affine *t = new(L, GS_DRAW_OBJ) trans::affine(obj);
  mlua_set_fenv_ref (L, objindex);

  t->rotate(a);

  return 1;
}

void
parse_spec (lua_State *L)
{
  const int specindex = 3, objindex = 2;
  const char *tag;
  const struct luaL_Reg *builder;

  lua_rawgeti (L, specindex, 1);
  if (! lua_isstring (L, -1))
    {
      luaL_error (L, "the tag of the transformation is invalid");
      return;
    }

  tag = lua_tostring (L, -1);
  lua_pop (L, 1);

  builder = mlua_find_method (trans_builder, tag);

  if (builder)
    builder->func (L);
  else
    luaL_error (L, "error in definition of pre or post transforms");

  lua_replace (L, objindex);
  lua_pop (L, 1);
}

int
lparse_spec_pipeline (lua_State *L)
{
  size_t k, nb;

  if (lua_type (L, 1) == LUA_TTABLE)
    nb = lua_objlen (L, 1);
  else
    return luaL_error (L, "post transform argument should be a table");

  for (k = nb; k > 0; k--)
    {
      lua_rawgeti (L, 1, k);
      parse_spec (L);
    }

  return 1;
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
      
  if (narg > 4)
    {
      lua_pushcfunction (L, lparse_spec_pipeline);
      lua_pushvalue (L, 5);
      lua_pushvalue (L, 2);
      lua_call (L, 2, 1);
      lua_replace (L, 2);
    }
    
  vertex_source *curr = check_agg_obj (L, 2);
  if (curr->need_resize())
    {
      new(L, GS_DRAW_OBJ) trans::resize(curr);
      mlua_set_fenv_ref (L, 2);
      lua_replace (L, 2);
    }

  if (narg > 3)
    {
      lua_pushcfunction (L, lparse_spec_pipeline);
      lua_pushvalue (L, 4);
      lua_pushvalue (L, 2);
      lua_call (L, 2, 1);
      lua_replace (L, 2);
    }
  
  curr = check_agg_obj (L, 2);

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
	  return luaL_error(L, "error creating thread.");
	}
      
      p->is_shown = 1;
      pthread_attr_destroy (attr);
    }

  AGG_UNLOCK();

  return 0;
}

int
window_debug_list(lua_State *L)
{
  lua_getfield (L, LUA_REGISTRYINDEX, "GSL.windows");
  return 1;
}

int
obj_getfenv(lua_State *L)
{
  lua_getfenv (L, 1);
  return 1;
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
