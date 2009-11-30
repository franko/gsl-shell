
#include "units_cplot.h"
#include "cplot.h"

#include "cplot-cintfc.h"

extern int agg_main (int argc, char *argv[]);

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
      val = 80;
      p += 4;
    }

  if (strcmp (p, "red") == 0)
    c = agg::rgba8(val, 0, 0);
  else if (strcmp (p, "green") == 0)
    c = agg::rgba8(0, val, 0);
  else if (strcmp (p, "blue") == 0)
    c = agg::rgba8(0, 0, val);
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
  line *d = (line *) _d;
  p->add(d);
}

CDRAW* line_new(const char *color_str)
{
  agg::rgba8 c = color_lookup (color_str);
  line* ln = new line(c);
  return (CDRAW *) ln;
}


CDRAW* line_copy(CDRAW *_src)
{
  line* src = (line*) _src;
  line* ln = new line(*src);
  return (CDRAW *) ln;
}

void line_free (CDRAW* _d)
{
  line* ln = (line*) _d;
  delete ln;
}

void line_move_to (CDRAW* _d, double x, double y)
{
  line* ln = (line*) _d;
  agg::path_storage& p = ln->path;
  p.move_to(x, y);
}

void line_line_to (CDRAW* _d, double x, double y)
{
  line* ln = (line*) _d;
  agg::path_storage& p = ln->path;
  p.line_to(x, y);
}

void line_close (CDRAW* _d)
{
  line* ln = (line*) _d;
  agg::path_storage& p = ln->path;
  p.close_polygon();
}

int
agg_main (int argc, char *argv[])
{
  return 0;
}
