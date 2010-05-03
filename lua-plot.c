
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "lua.h"
#include "lauxlib.h"

#include "gs-types.h"
#include "common.h"
#include "gsl-shell.h"
#include "lua-utils.h"
#include "lua-plot-priv.h"
#include "xwin-show.h"
#include "colors.h"

extern void plot_register (lua_State *L);

pthread_mutex_t agg_mutex[1];

enum agg_type {
  AGG_PATH = 1,
  AGG_TEXT,
};

struct agg_obj {
  enum agg_type tag;
  CVERTSRC *vs;
};

struct path_cmd_reg {
  enum path_cmd_e id;
  const char *cmd;
  const char *signature;
};

extern int push_new_agg_obj   (lua_State *L, enum agg_type tag, CVERTSRC *vs);

static int agg_obj_free       (lua_State *L);
static int agg_obj_index      (lua_State *L);

static int agg_path_new       (lua_State *L);
static int agg_path_index     (lua_State *L);

static int agg_text_new       (lua_State *L);
static int agg_text_index     (lua_State *L);
static int agg_text_set_text  (lua_State *L);
static int agg_text_set_point (lua_State *L);
static int agg_text_rotate    (lua_State *L);

static int agg_plot_new        (lua_State *L);
static int agg_plot_show       (lua_State *L);
static int agg_plot_add        (lua_State *L);
static int agg_plot_update     (lua_State *L);
static int agg_plot_add_line   (lua_State *L);
static int agg_plot_free       (lua_State *L);

static int agg_rgb_new         (lua_State *L);
static int agg_rgba_new        (lua_State *L);

struct agg_obj*  check_agg_obj  (lua_State *L, int index);
path *           check_agg_path (lua_State *L, int index);
CTEXT *          check_agg_text (lua_State *L, int index);
struct agg_plot* check_agg_plot (lua_State *L, int index);

static struct path_cmd_reg cmd_table[] = {
  {CMD_MOVE_TO,  "move_to",  "ff"},
  {CMD_LINE_TO,  "line_to",  "ff"},
  {CMD_CLOSE,    "close",    ""},
  {CMD_ARC_TO,   "arc_to",   "fffbbff"},
  {CMD_CURVE3,   "curve3",   "ffff"},
  {CMD_CURVE4,   "curve4",   "ffffff"},
  {CMD_ERROR,    NULL, NULL}
};

static const struct luaL_Reg plot_functions[] = {
  {"path",     agg_path_new},
  {"text",     agg_text_new},
  {"rgba",     agg_rgba_new},
  {"rgb",      agg_rgb_new},
  {"plot",     agg_plot_new},
  {NULL, NULL}
};

static const struct luaL_Reg agg_vertex_source_methods[] = {
  {"__index",     agg_obj_index},
  {"__gc",        agg_obj_free},
  {NULL, NULL}
};

static const struct luaL_Reg agg_plot_methods[] = {
  {"show",        agg_plot_show       },
  {"add",         agg_plot_add        },
  {"addline",     agg_plot_add_line   },
  {"update",      agg_plot_update     },
  {"__gc",        agg_plot_free       },
  {NULL, NULL}
};

static const struct luaL_Reg rgba_methods[] = {
  {NULL, NULL}
};

static const struct luaL_Reg agg_text_methods[] = {
  {"set_point",   agg_text_set_point},
  {"set_text",    agg_text_set_text},
  {"rotate",      agg_text_rotate},
  {NULL, NULL}
};

int
push_new_agg_obj (lua_State *L, enum agg_type tag, CVERTSRC *vs)
{
  struct agg_obj *d = lua_newuserdata (L, sizeof (struct agg_obj));

  d->vs = vs;
  d->tag = tag;

  vertex_source_ref (d->vs);

  gs_set_metatable (L, GS_DRAW_OBJ);

  return 1;
}

int
agg_path_new (lua_State *L)
{
  CPATH *vs = path_new ();
  push_new_agg_obj (L, AGG_PATH, (CVERTSRC *) vs);

  if (lua_gettop (L) >= 2)
    {
      double x = gs_check_number (L, 1, FP_CHECK_NORMAL);
      double y = gs_check_number (L, 2, FP_CHECK_NORMAL);
      struct cmd_call_stack s[1];
      
      s->f[0] = x;
      s->f[1] = y;

      path_cmd (vs, CMD_MOVE_TO, s);
    }

  return 1;
}

struct agg_obj *
check_agg_obj (lua_State *L, int index)
{
  return gs_check_userdata (L, index, GS_DRAW_OBJ);
}

path *
check_agg_path (lua_State *L, int index)
{
  struct agg_obj *d = gs_check_userdata (L, index, GS_DRAW_OBJ);
  if (d->tag == AGG_PATH)
    return (path *) d->vs;
  luaL_error (L, "expected object of type 'path' as argument #%i", index);
  return NULL;
}

int
agg_obj_free (lua_State *L)
{
  struct agg_obj *d = check_agg_obj (L, 1);
#ifdef DEBUG_PLOT
  fprintf(stderr, "lua dispose drawable %p\n", d->vs);
#endif
  vertex_source_unref (d->vs);
  return 0;
}

static int
agg_path_cmd (lua_State *L)
{
  path *p = check_agg_path (L, 1);
  int id = lua_tointeger (L, lua_upvalueindex(1));
  const char *signature = lua_tostring (L, lua_upvalueindex(2));
  int argc = 2, f_count = 0, b_count = 0;
  struct cmd_call_stack s[1];
  const char *fc;

  for (fc = signature; fc[0]; fc++)
    {
      switch (fc[0])
	{
	case 'f':
	  s->f[f_count++] = gs_check_number (L, argc++, FP_CHECK_NORMAL);
	  break;
	case 'b':
	  if (lua_isboolean (L, argc))
	    s->b[b_count++] = lua_toboolean (L, argc++);
	  else
	    return luaL_error (L, "expected boolean for argument #%i", argc);
	}
    }

  pthread_mutex_lock (agg_mutex);
  path_cmd (p, id, s);
  pthread_mutex_unlock (agg_mutex);
  return 0;
}

static int
agg_obj_pcall (lua_State *L)
{
  int narg_out, narg_in = lua_gettop (L);
  int status;

  pthread_mutex_lock (agg_mutex);
  lua_pushvalue (L, lua_upvalueindex(1));
  lua_insert (L, 1);
  status = lua_pcall (L, narg_in, LUA_MULTRET, 0);
  pthread_mutex_unlock (agg_mutex);
  if (status != 0)
    {
#ifndef LUA_MODULE
      error_report (L, status);
#else
      luaL_error (L, "error in graphical object method");
#endif
      return 0;
    }
  narg_out = lua_gettop (L);
  return narg_out;
}


int
agg_obj_index (lua_State *L)
{
  struct agg_obj *d = check_agg_obj (L, 1);

  lua_getmetatable (L, 1);
  lua_rawgeti (L, -1, (int) d->tag);
  lua_remove (L, -2);
  lua_insert (L, 1);

  lua_call (L, 2, 1);

  if (d->tag == AGG_PATH)
    return 1;

  if (! lua_isnil (L, -1))
    {
      lua_pushcclosure (L, agg_obj_pcall, 1);
      return 1;
    }

  return 0;
}

int
agg_path_index (lua_State *L)
{
  struct path_cmd_reg *r;
  const char *key;

  if (! lua_isstring (L, 2))
    return 0;

  key = lua_tostring (L, 2);
  for (r = cmd_table; r->cmd; r++)
    {
      if (strcmp (key, r->cmd) == 0)
	break;
    }

  if (r->cmd)
    {
      lua_pushinteger (L, (int) r->id);
      lua_pushstring (L, r->signature);
      lua_pushcclosure (L, agg_path_cmd, 2);
      return 1;
    }

  return 0;
}

CTEXT *
check_agg_text (lua_State *L, int index)
{
  struct agg_obj *d = gs_check_userdata (L, index, GS_DRAW_OBJ);
  if (d->tag == AGG_TEXT)
    return (CTEXT *) d->vs;
  luaL_error (L, "expected object of type 'text' as argument #%i", index);
  return NULL;
}

int
agg_text_new (lua_State *L)
{
  double size  = luaL_optnumber (L, 1, 10.0);
  double width = luaL_optnumber (L, 2, 1.0);
  CVERTSRC *vs = (CVERTSRC *) text_new (size, width);
  push_new_agg_obj (L, AGG_TEXT, vs);
  return 1;
}

int
agg_text_set_text (lua_State *L)
{
  CTEXT *t = check_agg_text (L, 1);
  const char *text = luaL_checkstring (L, 2);
  text_set_text (t, text);
  return 0;
}

int
agg_text_set_point (lua_State *L)
{
  CTEXT *t = check_agg_text (L, 1);
  double x = luaL_checknumber (L, 2);
  double y = luaL_checknumber (L, 3);
  text_set_point (t, x, y);
  return 0;
}

int
agg_text_rotate (lua_State *L)
{
  CTEXT *t = check_agg_text (L, 1);
  double a = luaL_checknumber (L, 2);
  text_rotate (t, a);
  return 0;
};

int
agg_text_index (lua_State *L)
{
  const struct luaL_Reg *reg;
  const char *key;

  if (! lua_isstring (L, 2))
    return 0;

  key = lua_tostring (L, 2);
  reg = mlua_find_method (agg_text_methods, key);
  if (reg)
    {
      lua_pushcfunction (L, reg->func);
      return 1;
    }

  return 0;
}

struct agg_plot *
check_agg_plot (lua_State *L, int index)
{
  struct agg_plot **ptr = gs_check_userdata (L, index, GS_PLOT);
  return *ptr;
}

int
agg_plot_new (lua_State *L)
{
  lua_Integer use_units = 1;
  struct agg_plot **pptr = lua_newuserdata (L, sizeof(void *));
  struct agg_plot *p;

  if (lua_isboolean (L, 1))
    use_units = lua_toboolean (L, 1);

  p = emalloc (sizeof(struct agg_plot));
  *pptr = p;

  p->plot = plot_new (use_units);

  p->lua_is_owner = 1;
  p->is_shown = 0;
  p->window = NULL;

  gs_set_metatable (L, GS_PLOT);

  return 1;
}

void
agg_plot_destroy (struct agg_plot *p)
{
  plot_free (p->plot);
  free (p);
}

int
agg_plot_free (lua_State *L)
{
  struct agg_plot *p = check_agg_plot (L, 1);

#ifdef DEBUG_PLOT
  fprintf(stderr, "lua dispose plot %p", p->plot);
#endif

  pthread_mutex_lock (agg_mutex);

  assert (p->lua_is_owner);
  p->lua_is_owner = 0;

  if (! p->is_shown)
    {
#ifdef DEBUG_PLOT
      fprintf(stderr, ": destroying\n");
#endif
      agg_plot_destroy (p);
    }
#ifdef DEBUG_PLOT
  else
    {
      fprintf(stderr, ": plot is shown\n");
    }
#endif

  pthread_mutex_unlock (agg_mutex);
  return 0;
}

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

struct trans_spec *
parse_spec (lua_State *L, int index, struct trans_spec *spec)
{
  const char *tag;

  lua_rawgeti (L, index, 1);
  if (! lua_isstring (L, -1))
    {
      lua_pop (L, 1);
      return NULL;
    }

  tag = lua_tostring (L, -1);
  lua_pop (L, 1);

  if (strcmp (tag, "stroke") == 0)
    {
      struct stroke_spec *prop = &spec->content.stroke;
      const char *prop_key;
      prop->width = mlua_named_optnumber (L, index, "width", 1.0);

      prop_key = mlua_named_optstring (L, index, "cap", NULL);
      prop->line_cap = property_lookup (line_cap_properties, prop_key);

      prop_key = mlua_named_optstring (L, index, "join", NULL);
      prop->line_join = property_lookup (line_join_properties, prop_key);

      spec->tag = trans_stroke;
      return spec;
    }
  else if (strcmp (tag, "marker") == 0)
    {
      struct marker_spec *prop = &spec->content.marker;
      prop->size = mlua_named_optnumber (L, index, "size", 3.0);
      spec->tag = trans_marker;
      return spec;
    }
  else if (strcmp (tag, "dash") == 0)
    {
      struct dash_spec *prop = &spec->content.dash;
      prop->len[0] = mlua_named_optnumber (L, index, "a", 10.0);
      prop->len[1] = mlua_named_optnumber (L, index, "b", prop->len[0]);
      spec->tag = trans_dash;
      return spec;
    }
  else if (strcmp (tag, "curve") == 0)
    {
      spec->tag = trans_curve;
      return spec;
    }
  else if (strcmp (tag, "rotate") == 0)
    {
      double a = mlua_named_number (L, index, "angle");
      spec->tag = trans_rotate;
      spec->content.rotate.angle = a;
      return spec;
    }
  else if (strcmp (tag, "translate") == 0)
    {
      double x = mlua_named_number (L, index, "x");
      double y = mlua_named_number (L, index, "y");
      spec->tag = trans_translate;
      spec->content.translate.x = x;
      spec->content.translate.y = y;
      return spec;
    }
  return NULL;
}

int
lparse_spec_pipeline (lua_State *L)
{
  struct trans_spec *spec;
  size_t k, nb;

  if (lua_type (L, 1) == LUA_TTABLE)
    nb = lua_objlen (L, 1);
  else
    return luaL_error (L, "post transform argument should be an array");

  spec = lua_newuserdata (L, (nb+1) * sizeof(struct trans_spec));
  for (k = 0; k < nb; k++)
    {
      lua_rawgeti (L, 1, k+1);
      if (parse_spec (L, lua_gettop (L), spec + k) == NULL)
	return luaL_error (L, "error in definition of post transforms");
      lua_pop (L, 1);
    }

  spec[k].tag = trans_end;

  return 1;
}

static struct trans_spec *
push_empty_pipeline (lua_State *L)
{
  struct trans_spec *spec;
  spec = lua_newuserdata (L, sizeof(struct trans_spec));
  spec->tag = trans_end;
  return spec;
}

static void
check_color (lua_State *L, int index, struct color *c)
{
  if (lua_isnil (L, index))
    {
      set_color_default (c);
    }
  else if (lua_isstring (L, index))
    {
      const char *cstr = lua_tostring (L, index);
      color_lookup (c, cstr);
    }
  else
    {
      struct color *userc = gs_check_userdata (L, index, GS_RGBA_COLOR);
      *c = *userc; /* struct assignement is ok in ANSI C */
    }
}

static int
agg_plot_add_gener (lua_State *L, bool as_line)
{
  struct agg_plot *p = check_agg_plot (L, 1);
  struct agg_obj *d = check_agg_obj (L, 2);
  struct trans_spec *post, *pre;
  int narg = lua_gettop (L);
  struct color color[1];

  if (narg <= 2)
    set_color_default (color);
  else
    check_color (L, 3, color);

  if (narg > 3)
    {
      lua_pushcfunction (L, lparse_spec_pipeline);
      lua_pushvalue (L, 4);
      lua_call (L, 1, 1);
      post = lua_touserdata (L, -1);
    }
  else
    {
      post = push_empty_pipeline (L);
    }
      
  assert (post != NULL);
      
  if (narg > 4)
    {
      lua_pushcfunction (L, lparse_spec_pipeline);
      lua_pushvalue (L, 5);
      lua_call (L, 1, 1);
      pre = lua_touserdata (L, -1);
    }
  else
    {
      pre = push_empty_pipeline (L);
    }

  assert (pre != NULL);
  
  pthread_mutex_lock (agg_mutex);
#ifdef DEBUG_PLOT
  printf("acquired mutex\n");
#endif
  plot_add (p->plot, d->vs, color, post, pre, as_line);

#ifdef DEBUG_PLOT
  printf("drawing\n");
#endif
  if (p->window)
    update_callback (p->window);
  pthread_mutex_unlock (agg_mutex);
#ifdef DEBUG_PLOT
  printf("unlocked mutex\n");
#endif

  return 0;
}
 
int
agg_plot_add (lua_State *L)
{
  return agg_plot_add_gener (L, false);
}
 
int
agg_plot_update (lua_State *L)
{
  struct agg_plot *p = check_agg_plot (L, 1);
  pthread_mutex_lock (agg_mutex);
  if (p->window)
    update_callback (p->window);
  pthread_mutex_unlock (agg_mutex);
  return 0;
}
 
int
agg_plot_add_line (lua_State *L)
{
  return agg_plot_add_gener (L, true);
}

int
agg_plot_show (lua_State *L)
{
  struct agg_plot *cp = check_agg_plot (L, 1);
  pthread_t xwin_thread[1];
  pthread_attr_t attr[1];

  pthread_mutex_lock (agg_mutex);

  if (! cp->is_shown)
    {
      pthread_attr_init (attr);
      pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);

      if (pthread_create(xwin_thread, attr, xwin_thread_function, (void*) cp))
	{
	  pthread_attr_destroy (attr);
	  return luaL_error(L, "error creating thread.");
	}
      
      cp->is_shown = 1;
      pthread_attr_destroy (attr);
    }

  pthread_mutex_unlock (agg_mutex);

  return 0;
}

static unsigned int double2uint8 (double x)
{
  int u = x * 255.0;
  if (u > 255)
    u = 255;
  else if (u < 0)
    u = 0;
  return (unsigned int) u;
}

int
agg_rgba_new (lua_State *L)
{
  double r = luaL_checknumber (L, 1);
  double g = luaL_checknumber (L, 2);
  double b = luaL_checknumber (L, 3);
  double a = luaL_checknumber (L, 4);
  struct color *c = lua_newuserdata (L, sizeof(struct color));
  c->r = double2uint8 (r);
  c->g = double2uint8 (g);
  c->b = double2uint8 (b);
  c->a = double2uint8 (a);

  gs_set_metatable (L, GS_RGBA_COLOR);

  return 1;
}

int
agg_rgb_new (lua_State *L)
{
  double r = luaL_checknumber (L, 1);
  double g = luaL_checknumber (L, 2);
  double b = luaL_checknumber (L, 3);
  struct color *c = lua_newuserdata (L, sizeof(struct color));
  c->r = double2uint8 (r);
  c->g = double2uint8 (g);
  c->b = double2uint8 (b);
  c->a = 255;

  gs_set_metatable (L, GS_RGBA_COLOR);

  return 1;
}

void
plot_register (lua_State *L)
{
  pthread_mutex_init (agg_mutex, NULL);

  /* plot declaration */
  luaL_newmetatable (L, GS_METATABLE(GS_PLOT));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, agg_plot_methods);
  lua_pop (L, 1);

  /* line declaration */
  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_OBJ));
  lua_pushinteger (L, (int) AGG_PATH);
  lua_pushcfunction (L, agg_path_index);
  lua_settable (L, -3);
  lua_pushinteger (L, (int) AGG_TEXT);
  lua_pushcfunction (L, agg_text_index);
  lua_settable (L, -3);
  luaL_register (L, NULL, agg_vertex_source_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, GS_METATABLE(GS_RGBA_COLOR));
  luaL_register (L, NULL, rgba_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, plot_functions);
}
