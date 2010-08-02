
/* lua-draw.cpp
 * 
 * Copyright (C) 2009, 2010 Francesco Abbate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

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

static int agg_path_free      (lua_State *L);
static int agg_path_index     (lua_State *L);

static int agg_text_free      (lua_State *L);
static int agg_text_set_text  (lua_State *L);
static int agg_text_set_point (lua_State *L);
// static int agg_text_rotate    (lua_State *L);

static int agg_rgba_free      (lua_State *L);
static int agg_rgba_add       (lua_State *L);
static int agg_rgba_mul       (lua_State *L);
static int agg_rgba_set_alpha (lua_State *L);

static void path_cmd (draw::path *p, int cmd, struct cmd_call_stack *stack);

static struct path_cmd_reg cmd_table[] = {
  {CMD_MOVE_TO,  "move_to",  "ff"},
  {CMD_LINE_TO,  "line_to",  "ff"},
  {CMD_CLOSE,    "close",    ""},
  {CMD_ARC_TO,   "arc_to",   "fffbbff"},
  {CMD_CURVE3,   "curve3",   "ffff"},
  {CMD_CURVE4,   "curve4",   "ffffff"},
  {CMD_ERROR,    NULL, NULL}
};

static const struct luaL_Reg draw_functions[] = {
  {"path",     agg_path_new},
  {"text",     agg_text_new},
  {"rgba",     agg_rgba_new},
  {"rgb",      agg_rgb_new},
  {NULL, NULL}
};

static const struct luaL_Reg agg_path_methods[] = {
  {"__index",     agg_path_index},
  {"__gc",        agg_path_free},
  {NULL, NULL}
};

static const struct luaL_Reg rgba_methods[] = {
  {"__gc",        agg_rgba_free},
  {"__add",       agg_rgba_add },
  {"__mul",       agg_rgba_mul },
  {"alpha",       agg_rgba_set_alpha },
  {NULL, NULL}
};

static const struct luaL_Reg agg_text_methods[] = {
  {"__gc",        agg_text_free},
  {"set_point",   agg_text_set_point},
  {"set_text",    agg_text_set_text},
  //  {"rotate",      agg_text_rotate},
  {NULL, NULL}
};

int
agg_path_new (lua_State *L)
{
  draw::path *vs = new(L, GS_DRAW_PATH) draw::path();

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

draw::path *
check_agg_path (lua_State *L, int index)
{
  return (draw::path *) gs_check_userdata (L, index, GS_DRAW_PATH);
}

int
agg_path_free (lua_State *L)
{
  typedef draw::path path_type;
  path_type *path = check_agg_path (L, 1);
  path->~path_type();
  return 0;
}

void
path_cmd (draw::path *p, int _cmd, struct cmd_call_stack *s)
{
  agg::path_storage& ps = p->self();
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
  draw::path *p = check_agg_path (L, 1);
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

/* NB: for the moment we have only one kind of "scalable" object. */
scalable *
check_agg_scalable (lua_State *L, int index)
{
  int tplist[] = {GS_DRAW_PATH, GS_INVALID_TYPE};
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

  return (scalable *) p;
}

draw::text *
check_agg_text (lua_State *L, int index)
{
  return (draw::text *) gs_check_userdata (L, index, GS_DRAW_TEXT);
}

int
agg_text_new (lua_State *L)
{
  double size  = luaL_optnumber (L, 1, 10.0);
  double width = luaL_optnumber (L, 2, 1.0);
  new(L, GS_DRAW_TEXT) draw::text(size, width);
  return 1;
}

int
agg_text_free (lua_State *L)
{
  typedef draw::text text_type;
  text_type *t = check_agg_text (L, 1);
  t->~text_type();
  return 0;
}

int
agg_text_set_text (lua_State *L)
{
  draw::text *t = check_agg_text (L, 1);
  const char *text = luaL_checkstring (L, 2);
  t->self().text(text);
  return 0;
}

int
agg_text_set_point (lua_State *L)
{
  draw::text *t = check_agg_text (L, 1);
  double x = luaL_checknumber (L, 2);
  double y = luaL_checknumber (L, 3);
  t->self().start_point(x, y);
  return 0;
}

/*
int
agg_text_rotate (lua_State *L)
{
  draw::text *t = check_agg_text (L, 1);
  double a = luaL_checknumber (L, 2);
  t->rotate(a);
  return 0;
};
*/

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
agg_rgba_set_alpha (lua_State *L)
{
  agg::rgba8 *c = (agg::rgba8 *) gs_check_userdata (L, 1, GS_RGBA_COLOR);
  double a = luaL_checknumber (L, 2);
  c->a = agg::rgba8::base_mask * a;
  return 0;
}

int
agg_rgba_add (lua_State *L)
{
  agg::rgba8 *c1 = (agg::rgba8 *) gs_check_userdata (L, 1, GS_RGBA_COLOR);
  agg::rgba8 *c2 = (agg::rgba8 *) gs_check_userdata (L, 2, GS_RGBA_COLOR);

  unsigned int r = c1->r + c2->r;
  unsigned int g = c1->g + c2->g;
  unsigned int b = c1->b + c2->b;

  new(L, GS_RGBA_COLOR) agg::rgba8(r, g, b);

  return 1;
}

int
agg_rgba_mul (lua_State *L)
{
  int is = 1, ic = 2;

  if (gs_is_userdata (L, 1, GS_RGBA_COLOR))
    {
      ic = 1;
      is = 2;
    }

  double f = luaL_checknumber (L, is);
  agg::rgba8 *c = (agg::rgba8 *) gs_check_userdata (L, ic, GS_RGBA_COLOR);

  unsigned int r = f * c->r, g = f * c->g, b = f * c->b;

  new(L, GS_RGBA_COLOR) agg::rgba8(r, g, b);
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

  markers::init();

  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_PATH));
  luaL_register (L, NULL, agg_path_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_TEXT));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, agg_text_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, GS_METATABLE(GS_RGBA_COLOR));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, rgba_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, draw_functions);
}
