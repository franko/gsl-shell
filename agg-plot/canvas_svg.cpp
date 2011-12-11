#include "canvas_svg.h"

const double canvas_svg::default_stroke_width = 1.0;

static void canvas_draw_svg(svg_vs* vs, FILE* f, int id, agg::rgba8 c)
{
  str s = vs->write_svg(id, c);
  canvas_svg::writeln(f, s, "   ");
}

static void canvas_draw_outline_svg(svg_vs* vs, FILE* f, int id, agg::rgba8 c)
{
  str path;
  svg_property_list* ls = vs->svg_path(path);
  str s = svg_stroke_path(path, canvas_svg::default_stroke_width, id, c, ls);
  list::free(ls);
  canvas_svg::writeln(f, s, "   ");
}

template <>
void canvas_svg::draw<svg_vs>(svg_vs& vs, agg::rgba8 c)
{
  canvas_draw_svg(&vs, m_output, m_current_id++, c);
}

template <>
void canvas_svg::draw_outline<svg_vs>(svg_vs& vs, agg::rgba8 c)
{
  canvas_draw_outline_svg(&vs, m_output, m_current_id++, c);
}

template <>
void canvas_svg::draw<drawable>(drawable& vs, agg::rgba8 c)
{
  canvas_draw_svg(&vs, m_output, m_current_id++, c);
}

template <>
void canvas_svg::draw_outline<drawable>(drawable& vs, agg::rgba8 c)
{
  canvas_draw_outline_svg(&vs, m_output, m_current_id++, c);
}
