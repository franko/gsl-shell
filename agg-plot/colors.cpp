
#include <string.h>

#include "lua-cpp-utils.h"
#include "colors.h"

static agg::rgba8
rgba8_lookup (lua_State *L, const char *color_str)
{
  const char *p = color_str;
  const int a = 255;

  if (strcmp (p, "white") == 0)
    return agg::rgba8(255, 255, 255, a);
  
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

  return agg::rgba8(r, g, b, a);
}

agg::rgba8
color_arg_lookup (lua_State *L, int index)
{
  if (lua_isnoneornil (L, index))
    return colors::cdefault;
  
  if (lua_isnumber(L, index))
    {
      unsigned int col = (unsigned int) lua_tointeger (L, index);
      agg::int8u r = (col & 0xff000000) >> 24;
      agg::int8u g = (col & 0x00ff0000) >> 16;
      agg::int8u b = (col & 0x0000ff00) >> 8;
      agg::int8u a = (col & 0x000000ff);

      return agg::rgba8(r, g, b, a);
    }

  const char *cstr = lua_tostring (L, index);

  if (!cstr)
    luaL_error (L, "invalid color specification");

  return rgba8_lookup (L, cstr);
}

agg::rgba colors::white(1, 1, 1);
agg::rgba colors::black(0, 0, 0);

agg::rgba8 colors::cdefault(180, 0, 0, 255);
