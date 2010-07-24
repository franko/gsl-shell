
#include <string.h>

#include "lua-cpp-utils.h"
#include "colors.h"

agg::rgba8 *
rgba8_push_default (lua_State *L)
{
  return new(L, GS_RGBA_COLOR) agg::rgba8(80, 0, 0, 255);
}

agg::rgba8 *
rgba8_push_lookup (lua_State *L, const char *color_str)
{
  const char *p = color_str;
  const int a = 255;

  if (strcmp (p, "white") == 0)
    return new(L, GS_RGBA_COLOR) agg::rgba8(255, 255, 255, a);
  
  int val = 180;
  int r = 0, g = 0, b = 0;

  if (strncmp (p, "light", 5) == 0)
    {
      val = 255;
      p += 5;
    }
  else if (strncmp (p, "dark", 4) == 0)
    {
      val = 120;
      p += 4;
    }

  if (strcmp (p, "red") == 0)
    r = val;
  else if (strcmp (p, "green") == 0)
    g = val;
  else if (strcmp (p, "blue") == 0)
    b = val;
  else if (strcmp (p, "cyan") == 0)
    g = b = val;
  else if (strcmp (p, "magenta") == 0)
    r = b = val;
  else if (strcmp (p, "yellow") == 0)
    r = g = val;
  else if (strcmp (p, "gray") == 0)
    r = g = b = val;

  return new(L, GS_RGBA_COLOR) agg::rgba8(r, g, b, a);
}

agg::rgba8 *
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
      c = (agg::rgba8 *) gs_check_userdata (L, index, GS_RGBA_COLOR);
    }

  return c;
}

agg::rgba8 *
check_color_rgba8  (lua_State *L, int index)
{
  return (agg::rgba8 *) gs_check_userdata (L, index, GS_RGBA_COLOR);
}

agg::rgba colors::white(1, 1, 1);
agg::rgba colors::black(0, 0, 0);
