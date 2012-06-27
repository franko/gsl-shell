#ifndef AGGPLOT_TEXT_SHAPE_H
#define AGGPLOT_TEXT_SHAPE_H

#include "agg_gsv_text.h"

#include "sg_object.h"

typedef sg_object_gen<agg::gsv_text> sg_text_base;

namespace draw {

  class text_shape : public sg_object  {
  public:
    text_shape(double x, double y, const char* text,
               double _size = 10.0, double hjustif = 0.0, double vjustif = 0.0):
      m_sg_text(), m_x(x), m_y(y), m_text(text), m_size(_size),
      m_scaling(0), m_trans(m_sg_text, identity_matrix), m_stroke(m_trans)
    {
      agg::gsv_text& t = m_sg_text.self();
      t.text(text);
      t.size(_size);

      const double text_width = t.text_width(), text_height = _size;
      m_x -= hjustif * text_width;
      m_y -= vjustif * text_height;

      t.start_point(m_x, m_y);

      m_stroke.line_cap(agg::round_cap);
      m_stroke.line_join(agg::round_join);
    }

    virtual void rewind(unsigned path_id)
    {
      m_sg_text.self().start_point(m_x, m_y);
      m_stroke.rewind(path_id);
    }

    virtual unsigned vertex(double* x, double* y)
    {
      return m_stroke.vertex(x, y);
    }

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
      agg::bounding_rect_single(m_sg_text, 0, x1, y1, x2, y2);
      const double pad = 1.0;
      *x1 -= pad;
      *y1 -= pad;
      *x2 += pad;
      *y2 += pad;
    }

    virtual str write_svg(int id, agg::rgba8 c, double h)
    {
      const char* text = m_text.cstr();
      int txt_size = (int)(m_size * 1.5);

      const agg::trans_affine& m = *m_scaling;

      double x = m_x, y = -m_y;
      const double dx = m.tx, dy = svg_y_coord(m.ty, h);

      if (is_unit_matrix(m))
        {
          x += dx;
          y += dy;
        }

      str svgtext = str::print("<text x=\"%g\" y=\"%g\" id=\"text%i\""        \
                               " style=\"font-size:%i\">"                \
                               " <tspan id=\"tspan%i\">%s</tspan>"        \
                               "</text>",
                               x, y, id, txt_size, id, text);

      str s;
      if (is_unit_matrix(m))
        s = svgtext;
      else
        s = str::print("<g transform=\"matrix(%g,%g,%g,%g,%g,%g)\">%s</g>",
                       m.sx, m.shx, m.shy, m.sy, dx, dy, svgtext.cstr());

      return s;
    }

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
      m_stroke.width(1.5 * m.scale());
      m_trans.transformer(m);
      m_scaling = &m;
    }

  private:
    sg_text_base m_sg_text;
    double m_x, m_y;
    str m_text;
    double m_size;

    const agg::trans_affine* m_scaling;
    agg::conv_transform<sg_object> m_trans;
    agg::conv_stroke<agg::conv_transform<sg_object> > m_stroke;
  };
}

#endif
