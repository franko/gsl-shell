#ifndef CANVAS_SVG_H
#define CANVAS_SVG_H

#include <stdio.h>

#include <agg_trans_affine.h>
#include <agg_color_rgba.h>

#include "defs.h"
#include "strpp.h"
#include "sg_object.h"
#include "draw_svg.h"

static const char *svg_header =                                                \
        "<?xml version=\"1.0\" standalone=\"no\"?>\n"                                \
        "<!-- Created using GSL Shell -->\n"                                        \
        "<svg\n"                                                                \
        "   xmlns=\"http://www.w3.org/2000/svg\"\n"                                \
        "   version=\"1.1\"\n"                                                \
        "   width=\"%g\"\n"                                                        \
        "   height=\"%g\"\n"                                                        \
        "   font-family=\"Helvetica\">\n";

static const char *svg_end = "</svg>\n";

class canvas_svg {
public:
    canvas_svg(FILE *f, double height):
        m_output(f), m_height(height), m_current_id(0)  { }

    void clip_box(const agg::rect_base<int>& clip) { }

    void reset_clipping() { }

    template <class VertexSource>
    void draw(VertexSource& vs, agg::rgba8 c)
    {
        str path;
        svg_coords_from_vs(&vs, path, m_height);
        str s = svg_fill_path(path, m_current_id++, c);
        writeln(m_output, s, "   ");
    }

    template <class VertexSource>
    void draw_outline(VertexSource& vs, agg::rgba8 c)
    {
        str path;
        svg_coords_from_vs(&vs, path, m_height);
        str s = svg_stroke_path(path, default_stroke_width, m_current_id++, c);
        writeln(m_output, s, "   ");
    }

    void write_header(double w, double h) {
        fprintf(m_output, svg_header, w, h);
    }
    void write_end() {
        fputs(svg_end, m_output);
    }

    void write_group_header(const char* id) {
        fprintf(m_output, "<g id=\"%s\">\n", id);
    }

    void write_group_end(const char* id) {
        fprintf(m_output, "</g>\n");
    }

    static void writeln(FILE* f, str& s, const char* indent = 0) {
        if (str_is_null(&s))
            return;
        if (indent)
            fputs(indent, f);
        fprintf(f, "%s\n", s.cstr());
    }

    static const double default_stroke_width;

private:
    FILE *m_output;
    double m_height;
    int m_current_id;
};

template <> void canvas_svg::draw<sg_object>(sg_object& vs, agg::rgba8 c);
template <> void canvas_svg::draw_outline<sg_object>(sg_object& vs, agg::rgba8 c);

#endif
