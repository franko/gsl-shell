
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_pixfmt_rgb.h"
#include "agg_trans_affine.h"

#include "text.h"

namespace draw {

#if 0
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
#endif

  void
  text::apply_transform(const agg::trans_affine& m, double as)
  {
    m_user_matrix.tx = m_x;
    m_user_matrix.ty = m_y;

    m.transform(&m_user_matrix.tx, &m_user_matrix.ty);

    m_matrix = m_user_matrix;

    m_trans.transformer(m_matrix);
    m_stroke.approximation_scale(as);
  }

  void
  text::bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    *x1 = *x2 = m_x;
    *y1 = *y2 = m_y;
  }

  str
  text::write_svg(int id, agg::rgba8 c, double h)
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

    bool need_rotate = !is_unit_matrix(m, eps);

    int txt_size = (int)(m_text_height * 1.5);

    double x = 0.0, y = - m_vjustif * m_text_height * 1.2;

    if (!need_rotate) {
      x = x + m.tx;
      y = svg_y_coord(y + m.ty, h);
    } else {
      y = -y;
    }

    const char* cont = m_text_buf.cstr();
    str txt = str::print("<text x=\"%g\" y=\"%g\" id=\"text%i\""        \
                         " style=\"font-size:%i%s\">"                        \
                         " <tspan id=\"tspan%i\">%s</tspan>" \
                         "</text>",
                         x, y, id, txt_size, style.cstr(),
                         id, cont);

    if (need_rotate) {
      s = str::print("<g transform=\"matrix(%g,%g,%g,%g,%g,%g)\">%s</g>",
                     m.sx, m.shx, m.shy, m.sy, m.tx, svg_y_coord(m.ty, h),
                     txt.cstr());
    } else {
      s = txt;
    }

    return s;
  }

#if 0
  bool text::render(pixel_type::lcd_fmt& pixbuf,
                    agg::rasterizer_scanline_aa<>& ras,
                    agg::scanline_u8& sl, agg::rgba8 c)
  {
    typedef agg::renderer_base<pixel_type::lcd_fmt> renderer_type;

    renderer_type ren_base(pixbuf);
    agg::renderer_scanline_aa_solid<renderer_type> ren_solid(ren_base);

    const char *text = m_text_buf.cstr();
    unsigned text_length = m_text_buf.len();
    double text_width = m_font_ren.text_width(text, text_length);
    double x = -m_hjustif * text_width;
    double y = -m_vjustif * m_text_height;

    m_font_ren.draw_text(ras, sl, ren_solid, m_matrix, x, y,
                         text, text_length, c);

    return true;
  }
#endif
}
