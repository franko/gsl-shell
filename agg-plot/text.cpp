
#include "agg_trans_affine.h"

#include "text.h"

namespace draw {

  void
  text::rewind(unsigned path_id)
  {
    m_text.start_point (-m_hjustif * m_text_width, -m_vjustif * m_text_height);
    m_stroke.rewind(path_id);
  }

  unsigned
  text::vertex(double* x, double* y)
  {
    return m_stroke.vertex(x, y);
  }

  void
  text::apply_transform(const agg::trans_affine& m, double as)
  {
    m_user_matrix.tx = m_x;
    m_user_matrix.ty = m_y;

    m.transform(&m_user_matrix.tx, &m_user_matrix.ty);

    m_matrix = m_user_matrix;

    if (m.sy < 0.0) {
      m_matrix.shy *= -1.0;
      m_matrix.sy  *= -1.0;
    }

    m_stroke.approximation_scale(as);
  }

  void
  text::bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    *x1 = *x2 = m_x;
    *y1 = *y2 = m_y;
  }

  str
  text::write_svg(int id, agg::rgba8 c) 
  {
    const agg::trans_affine& m = m_user_matrix;
    const double eps = 1.0e-6;
    str s;

    if (str_is_null(&m_text_buf))
      return s;

    str style;
    int hjust = lrint(m_hjustif * 2.0);
    if (hjust == 1)
      style.append(";text-anchor:middle");
    else if (hjust >= 2)
      style.append(";text-anchor:end");

    if (c.r != 0 || c.g != 0 || c.b != 0) {
      char rgbstr[8];
      format_rgb(rgbstr, c);
      style.printf_add(";fill:%s", rgbstr);
    }

    bool need_rotate = (fabs(m.sx - 1.0) > eps || fabs(m.shx) > eps || \
			fabs(m.shy) > eps || fabs(m.sy - 1.0) > eps);

    int txt_size = (int)(m_text_height * 1.5);

    double x = 0.0, y = m_vjustif * m_text_height * 1.2;
    if (!need_rotate) {
      x += m.tx;
      y += m.ty;
    }

    const char* cont = m_text_buf.cstr();
    str txt = str::print("<text x=\"%g\" y=\"%g\" id=\"text%i\""	\
			 " style=\"font-size:%i%s\">"			\
			 " <tspan x=\"%g\" y=\"%g\" id=\"tspan%i\">%s</tspan>" \
			 "</text>",
			 x, y, id, txt_size, style.cstr(), x, y, id, cont);

    if (need_rotate) {
      s = str::print("<g transform=\"matrix(%g,%g,%g,%g,%g,%g)\">%s</g>",
		     m.sx, m.shx, m.shy, m.sy, m.tx, m.ty, txt.cstr());
    } else {
      s = txt;
    }

    return s;
  }
}
