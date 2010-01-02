
#include <string.h>

#include "colors.h"
#include "agg_color_rgba.h"

void
set_color_default (struct color *c)
{
  c->r = 180;
  c->g = c->b = 0;
  c->a = 255;
}

void
color_lookup (struct color *c, const char *color_str)
{
  const char *p = color_str;
  int val = 180;

  c->a = 255;

  if (strcmp (p, "white") == 0)
    {
      c->r = c->g = c->b = 255;
      return;
    }

  c->r = c->g = c->b = 0;

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
    c->r = val;
  else if (strcmp (p, "green") == 0)
    c->g = val;
  else if (strcmp (p, "blue") == 0)
    c->b = val;
  else if (strcmp (p, "cyan") == 0)
    c->g = c->b = val;
  else if (strcmp (p, "magenta") == 0)
    c->r = c->b = val;
  else if (strcmp (p, "yellow") == 0)
    c->r = c->g = val;
  else if (strcmp (p, "gray") == 0)
    c->r = c->g = c->b = val;
}
