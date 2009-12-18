
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "lua.h"
#include "lauxlib.h"

#include "common.h"
#include "lua-utils.h"
#include "lua-plot-priv.h"
#include "xwin-show.h"

extern void plot_register (lua_State *L);

enum agg_type {
  AGG_PATH = 1,
  AGG_ELLIPSE,
  AGG_TEXT,
};

struct agg_obj {
  enum agg_type tag;
  vertex_source *vs;
};

struct path_cmd_reg {
  enum path_cmd_e id;
  const char *cmd;
  const char *signature;
};

static const char * const plot_mt_name          = "GSL.pl.plot";
static const char * const vertex_source_mt_name = "GSL.pl.vs";

static int agg_obj_free      (lua_State *L);
static int agg_obj_index     (lua_State *L);

static int agg_path_new      (lua_State *L);
static int agg_path_index    (lua_State *L);

static int agg_text_new      (lua_State *L);
static int agg_text_index    (lua_State *L);

static int agg_plot_new      (lua_State *L);
static int agg_plot_show     (lua_State *L);
static int agg_plot_add      (lua_State *L);
static int agg_plot_add_line (lua_State *L);
static int agg_plot_free     (lua_State *L);

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
  {"plot",     agg_plot_new},
  {NULL, NULL}
};

static const struct luaL_Reg agg_vertex_source_methods[] = {
  {"__index",     agg_obj_index},
  {"__gc",        agg_obj_free},
  {NULL, NULL}
};

static const struct luaL_Reg agg_plot_methods[] = {
  {"show",        agg_plot_show},
  {"add",         agg_plot_add},
  {"add_line",    agg_plot_add_line},
  {"__gc",        agg_plot_free},
  {NULL, NULL}
};

int
agg_path_new (lua_State *L)
{
  struct agg_obj *d = lua_newuserdata (L, sizeof (struct agg_obj));

  d->vs = (vertex_source *) path_new ();
  d->tag = AGG_PATH;

  vertex_source_ref (d->vs);

  luaL_getmetatable (L, vertex_source_mt_name);
  lua_setmetatable (L, -2);

  return 1;
}

struct agg_obj *
check_agg_obj (lua_State *L, int index)
{
  return (struct agg_obj *) luaL_checkudata (L, index, vertex_source_mt_name);
}

path *
check_agg_path (lua_State *L, int index)
{
  struct agg_obj *d = luaL_checkudata (L, index, vertex_source_mt_name);
  if (d->tag == AGG_PATH)
    return (path *) d->vs;
  luaL_error (L, "expected object of type 'path' as argument #%i", index);
  return NULL;
}

int
agg_obj_free (lua_State *L)
{
  struct agg_obj *d = check_agg_obj (L, 1);
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
	  s->f[f_count++] = luaL_checknumber (L, argc++);
	  break;
	case 'b':
	  if (lua_isboolean (L, argc))
	    s->b[b_count++] = lua_toboolean (L, argc++);
	  else
	    return luaL_error (L, "expected boolean for argument #%i", argc);
	}
    }

  path_cmd (p, id, s);
  return 0;
}

static int
agg_path_cmd_makeclosure (lua_State *L)
{
  lua_pushcclosure (L, agg_path_cmd, 2);
  return 1;
}

int
agg_obj_index (lua_State *L)
{
  struct agg_obj *d = check_agg_obj (L, 1);
  int narg;

  lua_getmetatable (L, 1);
  lua_rawgeti (L, -1, (int) d->tag);
  lua_remove (L, -2);
  lua_insert (L, 1);

  lua_call (L, 2, LUA_MULTRET);
  narg = lua_gettop (L);

  return narg;
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
      lua_pushcfunction (L, agg_path_cmd_makeclosure);
      lua_pushinteger (L, (int) r->id);
      lua_pushstring (L, r->signature);
      lua_call (L, 2, 1);
      return 1;
    }

  return 0;
}

CTEXT *
check_agg_text (lua_State *L, int index)
{
  struct agg_obj *d = luaL_checkudata (L, index, vertex_source_mt_name);
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
  struct agg_obj *d = lua_newuserdata (L, sizeof (struct agg_obj));

  d->vs = (vertex_source *) text_new (size, width);
  d->tag = AGG_TEXT;

  vertex_source_ref (d->vs);

  luaL_getmetatable (L, vertex_source_mt_name);
  lua_setmetatable (L, -2);

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
agg_text_index (lua_State *L)
{
  const char *key;

  if (! lua_isstring (L, 2))
    return 0;

  key = lua_tostring (L, 2);
  if (strcmp (key, "set_point") == 0)
    {
      lua_pushcfunction (L, agg_text_set_point);
      return 1;
    }
  if (strcmp (key, "set_text") == 0)
    {
      lua_pushcfunction (L, agg_text_set_text);
      return 1;
    }

  return 0;
}

struct agg_plot *
check_agg_plot (lua_State *L, int index)
{
  struct agg_plot **ptr = luaL_checkudata (L, index, plot_mt_name);
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
  pthread_mutex_init (p->mutex, NULL);

  p->lua_is_owner = 1;
  p->is_shown = 0;
  p->window = NULL;

  luaL_getmetatable (L, plot_mt_name);
  lua_setmetatable (L, -2);

  return 1;
}

void
agg_plot_destroy (struct agg_plot *p)
{
  plot_free (p->plot);
  pthread_mutex_destroy (p->mutex);
  free (p);
}

int
agg_plot_free (lua_State *L)
{
  struct agg_plot *p = check_agg_plot (L, 1);

  pthread_mutex_lock (p->mutex);

  assert (p->lua_is_owner);
  p->lua_is_owner = 0;

  if (! p->is_shown)
    {
      pthread_mutex_unlock (p->mutex);
      agg_plot_destroy (p);
      return 0;
    }

  pthread_mutex_unlock (p->mutex);
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
  else if (strcmp (tag, "curve") == 0)
    {
      spec->tag = trans_curve;
      return spec;
    }
  else if (strcmp (tag, "dash") == 0)
    {
      struct dash_spec *prop = &spec->content.dash;
      double a = mlua_named_optnumber (L, index, "a", 8.0);
      double b = mlua_named_optnumber (L, index, "b", a);
      prop->len[0] = a;
      prop->len[1] = b;
      spec->tag = trans_dash;
      return spec;
    }

  return NULL;
}

struct trans_spec *
parse_spec_pipeline (lua_State *L, int index)
{
  size_t k, nb;
  struct trans_spec *spec;

  if (lua_gettop (L) < index)
    {
      spec = emalloc (sizeof(struct trans_spec));
      spec->tag = trans_end;
      return spec;
    }

  if (lua_type (L, index) != LUA_TTABLE)
    return NULL;
  nb = lua_objlen (L, index);

  spec = emalloc ((nb+1) * sizeof(struct trans_spec));
  for (k = 0; k < nb; k++)
    {
      int subindex;
      lua_rawgeti (L, index, k+1);
      subindex = lua_gettop (L);
      if (parse_spec (L, subindex, spec + k) == NULL)
	{
	  free (spec);
	  return NULL;
	}
      lua_pop (L, 1);
    }

  spec[k].tag = trans_end;

  return spec;
}

static int
agg_plot_add_gener (lua_State *L, bool as_line)
{
  struct agg_plot *p = check_agg_plot (L, 1);
  struct agg_obj *d = check_agg_obj (L, 2);
  const char *color = luaL_checkstring (L, 3);
  
  if (as_line)
    {
      pthread_mutex_lock (p->mutex);
      plot_add_line (p->plot, d->vs, color);
    }
  else
    {
      struct trans_spec *post, *pre;
      
      post = parse_spec_pipeline (L, 4);
      if (post == NULL)
	luaL_error (L, "error in definition of post transforms");
      
      pre = parse_spec_pipeline (L, 5);
      if (pre == NULL)
	{
	  free (post);
	  luaL_error (L, "error in definition of pre transforms");
	}

      pthread_mutex_lock (p->mutex);
      plot_add (p->plot, d->vs, color, post, pre);

      free (post);
      free (pre);
    }

  if (p->window)
    update_callback (p->window);
  pthread_mutex_unlock (p->mutex);

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
agg_plot_show (lua_State *L)
{
  struct agg_plot *cp = check_agg_plot (L, 1);
  pthread_t xwin_thread[1];
  pthread_attr_t attr[1];

  pthread_mutex_lock (cp->mutex);

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

  pthread_mutex_unlock (cp->mutex);

  return 0;
}

void
plot_register (lua_State *L)
{
  /* plot declaration */
  luaL_newmetatable (L, plot_mt_name);
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, agg_plot_methods);
  lua_pop (L, 1);

  /* line declaration */
  luaL_newmetatable (L, vertex_source_mt_name);
  lua_pushinteger (L, (int) AGG_PATH);
  lua_pushcfunction (L, agg_path_index);
  lua_settable (L, -3);
  lua_pushinteger (L, (int) AGG_TEXT);
  lua_pushcfunction (L, agg_text_index);
  lua_settable (L, -3);
  luaL_register (L, NULL, agg_vertex_source_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, plot_functions);
}
