
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include "lua-text.h"
#include "gs-types.h"
#include "lua-utils.h"
#include "lua-cpp-utils.h"

#include "text.h"

static int agg_text_new       (lua_State *L);
static int agg_text_free      (lua_State *L);
static int agg_text_set_point (lua_State *L);
static int agg_text_index     (lua_State *L);
static int agg_text_newindex  (lua_State *L);

static int agg_text_text_set   (lua_State *L);
static int agg_text_angle_set  (lua_State *L);
static int agg_text_justif_set (lua_State *L);

static int agg_text_text_get   (lua_State *L);
static int agg_text_angle_get  (lua_State *L);

static draw::text* check_agg_text  (lua_State *L, int index);

static const struct luaL_Reg text_functions[] = {
  {"text",     agg_text_new},
  {NULL, NULL}
};

static const struct luaL_Reg text_metatable[] = {
  {"__gc",        agg_text_free},
  {"__index",     agg_text_index},
  {"__newindex",  agg_text_newindex},
  {NULL, NULL}
};

static const struct luaL_Reg text_methods[] = {
  {"set",         agg_text_set_point},
  {"justif",      agg_text_justif_set  },
  {NULL, NULL}
};

static const struct luaL_Reg text_properties_get[] = {
  {"text",        agg_text_text_get  },
  {"angle",       agg_text_angle_get  },
  {NULL, NULL}
};

static const struct luaL_Reg text_properties_set[] = {
  {"text",        agg_text_text_set  },
  {"angle",       agg_text_angle_set  },
  {NULL, NULL}
};

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
agg_text_text_set (lua_State *L)
{
  draw::text *t = check_agg_text (L, 1);
  const char *str_text = luaL_checkstring (L, 2);
  t->set_text(str_text);
  return 0;
}

int
agg_text_angle_set (lua_State *L)
{
  draw::text *t = check_agg_text (L, 1);
  double th = luaL_checknumber (L, 2);
  t->angle(th);
  return 0;
}

int
agg_text_angle_get (lua_State *L)
{
  draw::text *t = check_agg_text (L, 1);
  lua_pushnumber (L, t->angle());
  return 1;
}

int
agg_text_text_get (lua_State *L)
{
  draw::text *t = check_agg_text (L, 1);
  lua_pushstring (L, t->get_text());
  return 1;
}

int
agg_text_justif_set (lua_State *L)
{
  draw::text *t = check_agg_text (L, 1);
  const char *justif = luaL_checkstring (L, 2);
  size_t len = strlen (justif);

  if (len > 0)
    {
      char ch = justif[0];
      double hjf;
      switch (ch)
	{
	case 'l':
	  hjf = 0.0;
	  break;
	case 'c':
	  hjf = 0.5;
	  break;
	case 'r':
	  hjf = 1.0;
	  break;
	default:
	  return luaL_error (L, "invalid text justification");
	}

      t->hjustif(hjf);
    }

  if (len > 1)
    {
      char ch = justif[1];
      double vjf;
      switch (ch)
	{
	case 'b':
	  vjf = 0.0;
	  break;
	case 'c':
	  vjf = 0.5;
	  break;
	case 't':
	  vjf = 1.0;
	  break;
	default:
	  return luaL_error (L, "invalid text justification");
	}

      t->vjustif(vjf);
    }

  return 0;
}

int
agg_text_set_point (lua_State *L)
{
  draw::text *t = check_agg_text (L, 1);
  double x = luaL_checknumber (L, 2);
  double y = luaL_checknumber (L, 3);
  t->set_point(x, y);
  return 0;
}

int
agg_text_index (lua_State *L)
{
  return mlua_index_with_properties (L, text_properties_get, text_methods, false);
}

int
agg_text_newindex (lua_State *L)
{
  return mlua_newindex_with_properties (L, text_properties_set);
}

void
text_register (lua_State *L)
{
  luaL_newmetatable (L, GS_METATABLE(GS_DRAW_TEXT));
  luaL_register (L, NULL, text_metatable);
  lua_pop (L, 1);

  luaL_register (L, NULL, text_functions);
}
