
#include <pthread.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "lua-draw.h"
#include "gsl-shell.h"
#include "lua-cpp-utils.h"
#include "gs-types.h"
#include "trans.h"
#include "colors.h"

pthread_mutex_t agg_mutex[1];

enum path_cmd_e {
  CMD_ERROR = -1,
  CMD_MOVE_TO = 0,
  CMD_LINE_TO,
  CMD_CLOSE,
  CMD_ARC_TO,
  CMD_CURVE3,
  CMD_CURVE4,
};

struct cmd_call_stack {
  double f[6];
  int b[2];
};

struct path_cmd_reg {
  enum path_cmd_e id;
  const char *cmd;
  const char *signature;
};

static int agg_obj_index      (lua_State *L);
static int agg_obj_free       (lua_State *L);

static int agg_path_free      (lua_State *L);
static int agg_path_index     (lua_State *L);

static int agg_text_free      (lua_State *L);
static int agg_text_set_text  (lua_State *L);
static int agg_text_set_point (lua_State *L);
static int agg_text_rotate    (lua_State *L);

static int agg_rgba_free      (lua_State *L);

static void path_cmd (my::path *p, int cmd, struct cmd_call_stack *stack);

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
  {NULL, NULL}
};

static const struct luaL_Reg agg_obj_methods[] = {
  {"__index",     agg_obj_index},
  {"__gc",        agg_obj_free},
  {NULL, NULL}
};

static const struct luaL_Reg agg_path_methods[] = {
  {"__index",     agg_path_index},
  {"__gc",        agg_path_free},
  {NULL, NULL}
};

static const struct luaL_Reg rgba_methods[] = {
  {"__gc",        agg_rgba_free},
  {NULL, NULL}
};

static const struct luaL_Reg agg_text_methods[] = {
  {"__gc",        agg_text_free},
  {"set_point",   agg_text_set_point},
  {"set_text",    agg_text_set_text},
  {"rotate",      agg_text_rotate},
  {NULL, NULL}
};

int
agg_path_new (lua_State *L)
{
  my::path *vs = new(L, GS_DRAW_PATH) my::path();

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

my::path *
check_agg_path (lua_State *L, int index)
{
  return (my::path *) gs_check_userdata (L, index, GS_DRAW_PATH);
}

int
agg_path_free (lua_State *L)
{
  typedef my::path path_type;
  path_type *path = check_agg_path (L, 1);
  printf("freeing PATH %p\n", (void *) path);
  path->~path_type();
  return 0;
}

void
path_cmd (my::path *p, int _cmd, struct cmd_call_stack *s)
{
  agg::path_storage& ps = p->get_path();
  path_cmd_e cmd = (path_cmd_e) _cmd;

  switch (cmd)
    {
    case CMD_MOVE_TO:
      ps.move_to (s->f[0], s->f[1]);
      break;
    case CMD_LINE_TO:
      ps.line_to (s->f[0], s->f[1]);
      break;
    case CMD_CLOSE:
      ps.close_polygon ();
      break;
    case CMD_ARC_TO:
      ps.arc_to (s->f[0], s->f[1], s->f[2], s->b[0], s->b[1], s->f[3], s->f[4]);
      break;
    case CMD_CURVE3:
      ps.curve3 (s->f[0], s->f[1], s->f[2], s->f[3]);
      break;
    case CMD_CURVE4:
      ps.curve4 (s->f[0], s->f[1], s->f[2], s->f[3], s->f[4], s->f[5]);
      break;
    case CMD_ERROR:
      ;
    }
}

static int
agg_path_cmd (lua_State *L)
{
  my::path *p = check_agg_path (L, 1);
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

vertex_source *
check_agg_obj (lua_State *L, int index)
{
  int tplist[] = {GS_DRAW_OBJ, GS_DRAW_PATH, GS_DRAW_TEXT, GS_INVALID_TYPE};
  void *p = NULL;
  int j;

  for (j = 0; tplist[j] != GS_INVALID_TYPE; j++)
    {
      p = gs_is_userdata (L, index, tplist[j]);
      if (p)
	break;
    }

  if (p == NULL)
    gs_type_error (L, index, "drawing object");

  return (vertex_source *) p;
}

int
agg_obj_free (lua_State *L)
{
  vertex_source *vs = check_agg_obj (L, 1);
  printf("freeing OBJECT %p\n", (void *) vs);
  vs->~vertex_source();
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
  vertex_source *vs = check_agg_obj (L, 1);

  lua_getmetatable (L, 1);
  lua_insert (L, 2);
  lua_gettable (L, 2);

  if (! lua_isnil (L, -1))
    {
      lua_pushcclosure (L, agg_obj_pcall, 1);
      return 1;
    }

  return 0;
}

my::text *
check_agg_text (lua_State *L, int index)
{
  return (my::text *) gs_check_userdata (L, index, GS_DRAW_TEXT);
}

int
agg_text_new (lua_State *L)
{
  double size  = luaL_optnumber (L, 1, 10.0);
  double width = luaL_optnumber (L, 2, 1.0);
  my::text *txt = new(L, GS_DRAW_TEXT) my::text(size, width);
  return 1;
}

int
agg_text_free (lua_State *L)
{
  typedef my::text text_type;
  text_type *t = check_agg_text (L, 1);
  t->~text_type();
  return 0;
}

int
agg_text_set_text (lua_State *L)
{
  my::text *t = check_agg_text (L, 1);
  const char *text = luaL_checkstring (L, 2);
  t->set_text(text);
  return 0;
}

int
agg_text_set_point (lua_State *L)
{
  my::text *t = check_agg_text (L, 1);
  double x = luaL_checknumber (L, 2);
  double y = luaL_checknumber (L, 3);
  t->start_point(x, y);
  return 0;
}

int
agg_text_rotate (lua_State *L)
{
  my::text *t = check_agg_text (L, 1);
  double a = luaL_checknumber (L, 2);
  t->rotate(a);
  return 0;
};

static unsigned int double2uint8 (double x)
{
  int u = x * 255.0;
  if (u > 255)
    u = 255;
  else if (u < 0)
    u = 0;
  return (unsigned int) u;
}

agg::rgba8 *
check_agg_rgba8 (lua_State *L, int index)
{
  return (agg::rgba8 *) gs_check_userdata (L, index, GS_RGBA_COLOR);
}

int
agg_rgba_new (lua_State *L)
{
  unsigned int r = double2uint8 (luaL_checknumber (L, 1));
  unsigned int g = double2uint8 (luaL_checknumber (L, 2));
  unsigned int b = double2uint8 (luaL_checknumber (L, 3));
  unsigned int a = double2uint8 (luaL_checknumber (L, 4));

  new(L, GS_RGBA_COLOR) agg::rgba8(r, g, b, a);
  return 1;
}

int
agg_rgb_new (lua_State *L)
{
  unsigned int r = double2uint8 (luaL_checknumber (L, 1));
  unsigned int g = double2uint8 (luaL_checknumber (L, 2));
  unsigned int b = double2uint8 (luaL_checknumber (L, 3));

  new(L, GS_RGBA_COLOR) agg::rgba8(r, g, b, 255);
  return 1;
}

int
agg_rgba_free (lua_State *L)
{
  typedef agg::rgba8 rgba_t;
  rgba_t *c = check_agg_rgba8 (L, 1);
  c->~rgba_t();
  return 0;
}

void
draw_register (lua_State *L)
{
  pthread_mutex_init (agg_mutex, NULL);

  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_PATH));
  luaL_register (L, NULL, agg_path_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_TEXT));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, agg_text_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_OBJ));
  luaL_register (L, NULL, agg_obj_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, GS_METATABLE(GS_RGBA_COLOR));
  luaL_register (L, NULL, rgba_methods);
  lua_pop (L, 1);
}
