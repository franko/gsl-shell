#ifndef DRAW_SVG_H
#define DRAW_SVG_H

#include "agg_basics.h"
#include "agg_color_rgba.h"
#include "my_list.h"
#include "strpp.h"

enum svg_path_property_e {
  stroke_dasharray = 0,
};

extern const char *svg_path_property_name[];

struct svg_property_item {
  enum svg_path_property_e key;
  const char* value;

  svg_property_item(enum svg_path_property_e k, const char *v) : key(k), value(v) { }
};

typedef pod_list<svg_property_item> svg_property_list;

template <typename VertexSource>
void svg_coords_from_vs(VertexSource* vs, str& s)
{
  unsigned cmd;
  double x, y;
  const char * const space = " ";
  const char *sep = "";

  vs->rewind(0);

  while ((cmd = vs->vertex(&x, &y)))
    {
      if (agg::is_move_to(cmd)) {
	s.printf_add("%sM %g,%g", sep, x, y);
      } else if (agg::is_line_to(cmd)) {
	s.printf_add("%s%g,%g", sep, x, y);
      }	else if (agg::is_close(cmd)) {
	s.printf_add("%sz", sep);
      }	else if (agg::is_curve3(cmd)) {
	vs->vertex(&x, &y);
	s.printf_add("%s%g,%g", sep, x, y);
      }	else if (agg::is_curve4(cmd)) {
	vs->vertex(&x, &y);
	vs->vertex(&x, &y);
	s.printf_add("%s%g,%g", sep, x, y);
      }
      sep = space;
    }
}

template <typename VertexSource>
void svg_curve_coords_from_vs(VertexSource* vs, str& s)
{
  unsigned cmd;
  double x, y;
  const char * const space = " ";
  const char *sep = "";
  bool omit_line_to = false;

  vs->rewind(0);

  while ((cmd = vs->vertex(&x, &y)))
    {
      if (agg::is_move_to(cmd)) {
	s.printf_add("%sM %g,%g", sep, x, y);
	omit_line_to = true;
      } else if (agg::is_line_to(cmd)) {
	s.printf_add("%s%s%g,%g", sep, omit_line_to ? "" : "L ", x, y);
      }	else if (agg::is_curve4(cmd)) {
	double x1 = x, y1 = y;
	double x2, y2;
	vs->vertex(&x2, &y2);
	vs->vertex(&x, &y);
	s.printf_add("%sC %g,%g %g,%g %g,%g", sep, x1, y1, x2, x2, x, y);
	omit_line_to = false;
      }	else if (agg::is_curve3(cmd)) {
	double x1 = x, y1 = y;
	vs->vertex(&x, &y);
	s.printf_add("%sQ %g,%g %g,%g", sep, x1, y1, x, y);
	omit_line_to = false;
      }	else if (agg::is_close(cmd)) {
	s.printf_add("%sz", sep);
      }
      sep = space;
    }
}

extern str svg_stroke_path(str& path_coords, double width, int id, agg::rgba8 c, svg_property_list* properties = 0);
extern str svg_fill_path(str& path_coords, int id, agg::rgba8 c, svg_property_list* properties = 0);
extern void format_rgb(char rgbstr[], agg::rgba8 c);

#endif
