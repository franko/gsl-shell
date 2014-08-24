#ifndef DRAW_SVG_H
#define DRAW_SVG_H

#include "agg_basics.h"
#include "agg_color_rgba.h"
#include "list.h"
#include "strpp.h"

enum svg_path_property_e {
    stroke_dasharray = 0,
    marker_start,
    marker_mid,
    marker_end,
};

extern const char *svg_path_property_name[];

struct svg_property_item {
    enum svg_path_property_e key;
    const char* value;

    svg_property_item(enum svg_path_property_e k, const char *v) : key(k), value(v) { }
};

static inline double svg_y_coord(double y, double h) {
    return h - y;
}

template <class VertexSource>
static inline unsigned
vertex_flip(VertexSource* vs, double* x, double* y, double h)
{
    unsigned cmd = vs->vertex(x, y);
    *y = svg_y_coord(*y, h);
    return cmd;
}

typedef list<svg_property_item> svg_property_list;

template <typename VertexSource>
void svg_coords_from_vs(VertexSource* vs, str& s, double h)
{
    unsigned cmd;
    double x, y;
    const char * const space = " ";
    const char *sep = "";

    vs->rewind(0);

    while ((cmd = vertex_flip(vs, &x, &y, h)))
    {
        if (agg::is_move_to(cmd)) {
            s.printf_add("%sM %g,%g", sep, x, y);
        } else if (agg::is_line_to(cmd)) {
            s.printf_add("%s%g,%g", sep, x, y);
        }        else if (agg::is_close(cmd)) {
            s.printf_add("%sz", sep);
        }        else if (agg::is_curve3(cmd)) {
            vertex_flip(vs, &x, &y, h);
            s.printf_add("%s%g,%g", sep, x, y);
        }        else if (agg::is_curve4(cmd)) {
            vs->vertex(&x, &y);
            vertex_flip(vs, &x, &y, h);
            s.printf_add("%s%g,%g", sep, x, y);
        }
        sep = space;
    }
}

template <typename VertexSource>
void svg_curve_coords_from_vs(VertexSource* vs, str& s, double h)
{
    unsigned cmd;
    double x, y;
    const char * const space = " ";
    const char *sep = "";
    bool omit_line_to = false;

    vs->rewind(0);

    while ((cmd = vertex_flip(vs, &x, &y, h)))
    {
        if (agg::is_move_to(cmd)) {
            s.printf_add("%sM %g,%g", sep, x, y);
            omit_line_to = true;
        } else if (agg::is_line_to(cmd)) {
            s.printf_add("%s%s%g,%g", sep, omit_line_to ? "" : "L ", x, y);
        }        else if (agg::is_curve4(cmd)) {
            double x1 = x, y1 = y;
            double x2, y2;
            vertex_flip(vs, &x2, &y2, h);
            vertex_flip(vs, &x, &y, h);
            s.printf_add("%sC %g,%g %g,%g %g,%g", sep, x1, y1, x2, y2, x, y);
            omit_line_to = false;
        }        else if (agg::is_curve3(cmd)) {
            double x1 = x, y1 = y;
            vertex_flip(vs, &x, &y, h);
            s.printf_add("%sQ %g,%g %g,%g", sep, x1, y1, x, y);
            omit_line_to = false;
        }        else if (agg::is_close(cmd)) {
            s.printf_add("%sz", sep);
        }
        sep = space;
    }
}

extern str svg_stroke_path(str& path_coords, double width, int id, agg::rgba8 c, svg_property_list* properties = 0);
extern str svg_fill_path(str& path_coords, int id, agg::rgba8 c, svg_property_list* properties = 0);
extern str svg_marker_path(str& path_coords, double sw, int id, svg_property_list* properties);
extern void format_rgb(char rgbstr[], agg::rgba8 c);

#endif
