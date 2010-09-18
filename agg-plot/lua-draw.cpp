
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

static int agg_ellipse_new    (lua_State *L);
static int agg_circle_new     (lua_State *L);
static int agg_ellipse_free   (lua_State *L);

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
  {"ellipse",  agg_ellipse_new},
  {"circle",   agg_circle_new},
  {"rgba",     agg_rgba_new},
  {"rgb",      agg_rgb_new},
  {NULL, NULL}
};

static const struct luaL_Reg agg_path_methods[] = {
  {"__index",     agg_path_index},
  {"__gc",        agg_path_free},
  {NULL, NULL}
};


static const struct luaL_Reg agg_ellipse_methods[] = {
  {"__gc",        agg_ellipse_free},
  {NULL, NULL}
};

static const struct luaL_Reg rgba_methods[] = {
  {"__gc",        agg_rgba_free},
  {"__add",       agg_rgba_add },
  {"__mul",       agg_rgba_mul },
  {"alpha",       agg_rgba_set_alpha },
  {NULL, NULL}
};

int
agg_path_new (lua_State *L)
{
  draw::path *vs = push_new_object<draw::path>(L, GS_DRAW_PATH);

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
  return object_free<draw::path>(L, 1, GS_DRAW_PATH);
}

#if 0
#warning DEBUG CODE
static const int DEBUG_error_count_max = 8 * 1024;
static int DEBUG_error_count = DEBUG_error_count_max;
#endif

void
path_cmd (draw::path *p, int _cmd, struct cmd_call_stack *s)
{
  agg::path_storage& ps = p->self();
  path_cmd_e cmd = (path_cmd_e) _cmd;

#if 0
  DEBUG_error_count--;
  if (DEBUG_error_count == 0)
    {
      DEBUG_error_count = DEBUG_error_count_max;
      throw std::bad_alloc();
    }
#endif

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
  try
    {
      path_cmd (p, id, s);
    }
  catch (std::bad_alloc&)
    {
      pthread_mutex_unlock (agg_mutex);
      luaL_error (L, "out of memory");
      return 0;
    }
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

int
agg_ellipse_new (lua_State *L)
{
  draw::ellipse *vs = push_new_object<draw::ellipse>(L, GS_DRAW_ELLIPSE);
  double x = luaL_checknumber (L, 1);
  double y = luaL_checknumber (L, 2);
  double rx = luaL_checknumber (L, 3);
  double ry = luaL_checknumber (L, 4);
  vs->self().init(x, y, rx, ry, 0, false);
  return 1;
}

int
agg_circle_new (lua_State *L)
{
  draw::ellipse *vs = push_new_object<draw::ellipse>(L, GS_DRAW_ELLIPSE);
  double x = luaL_checknumber (L, 1);
  double y = luaL_checknumber (L, 2);
  double r = luaL_checknumber (L, 3);
  vs->self().init(x, y, r, r, 0, false);
  return 1;
}

int
agg_ellipse_free (lua_State *L)
{
  return object_free<draw::ellipse>(L, 1, GS_DRAW_ELLIPSE);
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
  return object_free<agg::rgba8>(L, 1, GS_RGBA_COLOR);
}

void
draw_register (lua_State *L)
{
  pthread_mutex_init (agg_mutex, NULL);

  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_PATH));
  luaL_register (L, NULL, agg_path_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_ELLIPSE));
  luaL_register (L, NULL, agg_ellipse_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, GS_METATABLE(GS_RGBA_COLOR));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, rgba_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, draw_functions);
}
