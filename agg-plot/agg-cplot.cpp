
#include "units_cplot.h"
#include "lua-cplot-priv.h"
#include "cplot.h"

#include "agg-cplot.h"

static agg::rgba8
color_lookup (const char *color_str)
{
  const char *p = color_str;
  agg::rgba8 c;
  int val = 180;

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
    c = agg::rgba8(val, 0, 0);
  else if (strcmp (p, "green") == 0)
    c = agg::rgba8(0, val, 0);
  else if (strcmp (p, "blue") == 0)
    c = agg::rgba8(0, 0, val);
  else if (strcmp (p, "cyan") == 0)
    c = agg::rgba8(0, val, val);
  else if (strcmp (p, "magenta") == 0)
    c = agg::rgba8(val, 0, val);
  else if (strcmp (p, "yellow") == 0)
    c = agg::rgba8(val, val, 0);
  else if (strcmp (p, "gray") == 0)
    c = agg::rgba8(val, val, val);
  else
    c = agg::rgba8(0, 0, 0);

  return c;
}

CCPLOT *
cplot_new(int with_units)
{
  cplot *cp;
  if (with_units)
    cp = new units_cplot();
  else
    cp = new cplot();

  return (CCPLOT *) cp; 
}

void cplot_free (CCPLOT* _d)
{
  cplot* cp = (cplot*) _d;
  delete cp;
}

void cplot_add(CCPLOT *_p, CDRAW *_d)
{
  cplot *p = (cplot *) _p;
  drawable *d = (drawable *) _d;
  p->add(d);
}

CPATH* line_new(const char *color_str)
{
  agg::rgba8 c = color_lookup (color_str);
  line* ln = new line(c);
  return (CPATH *) ln;
}

CPATH* poly_new(const char *color_str, const char *outline_color_str)
{
  agg::rgba8 fill_col = color_lookup (color_str);
  line* ln;

  if (outline_color_str)
    {
      agg::rgba8 outline_col = color_lookup (outline_color_str);
      ln = new poly_outline(fill_col, outline_col);
    }
  else
    {
      ln = new polygon(fill_col);
    }

  return (CPATH *) ln;
}


CPATH* line_copy(CPATH *_src)
{
  line* src = (line*) _src;
  line* ln = new line(*src);
  return (CPATH *) ln;
}

void line_free (CPATH* _d)
{
  line* ln = (line*) _d;
  delete ln;
}

void
line_cmd (CPATH *_d, struct cmd_call_stack *s)
{
  line* ln = (line*) _d;
  agg::path_storage& p = ln->path;

  switch (s->cmd)
    {
    case CMD_MOVE_TO:
      p.move_to (s->f[0], s->f[1]);
      break;
    case CMD_LINE_TO:
      p.line_to (s->f[0], s->f[1]);
      break;
    case CMD_CLOSE:
      p.close_polygon ();
      break;
    case CMD_SET_DASH:
      ln->set_dash (s->f[0], s->f[1]);
      break;
    case CMD_ADD_DASH:
      ln->add_dash (s->f[0], s->f[1]);
      break;
    case CMD_ARC_TO:
      p.arc_to (s->f[0], s->f[1], s->f[2], s->b[0], s->b[1], s->f[3], s->f[4]);
      break;
    case CMD_CURVE3:
      p.curve3 (s->f[0], s->f[1], s->f[2], s->f[3]);
      break;
    case CMD_CURVE4:
      p.curve4 (s->f[0], s->f[1], s->f[2], s->f[3], s->f[4], s->f[5]);
    }
}

CDRAW *
ellipse_new (double x, double y, double rx, double ry)
{
	drawable* d = 
}

extern void     ellipse_free (CDRAW *e);


CDRAW *
drawable_copy (CDRAW *_d)
{
  drawable *src = (drawable *) _d;
  drawable *dst = src->copy();
  return (CDRAW *) dst;
}
