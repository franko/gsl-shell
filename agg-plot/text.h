#ifndef AGGPLOT_TEXT_H
#define AGGPLOT_TEXT_H

#include "agg_gsv_text.h"
#include "agg_trans_affine.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_renderer_scanline.h"
#include "agg_font_freetype.h"

#include "sg_object.h"

namespace draw {

  class text : public sg_object {

    enum { scale_x = 100 };

    typedef agg::font_engine_freetype_int32 font_engine_type;
    typedef agg::font_cache_manager<font_engine_type> font_manager_type;

    typedef agg::gsv_text vs_text;
    typedef agg::conv_transform<vs_text> vs_trans_text;
    typedef agg::conv_stroke<vs_trans_text> vs_stroked_text;

    agg::trans_affine m_matrix;
    agg::trans_affine m_user_matrix;

    str m_text_buf;
    vs_text m_text;
    vs_trans_text m_trans;
    vs_stroked_text m_stroke;

    double m_x, m_y;
    double m_angle;

    double m_text_width;
    double m_text_height;

    double m_hjustif;
    double m_vjustif;

    unsigned m_text_pos;
    // double m_text_start_x;
    double m_text_x, m_text_y;
    double m_advance_x, m_advance_y;
    // const agg::glyph_cache* m_glyph;
    // font_renderer::font_engine_type::path_adaptor_type m_text_path;

    font_engine_type m_font_eng;
    font_manager_type m_font_man;

    agg::trans_affine m_text_mtx;
    agg::conv_curve<font_manager_type::path_adaptor_type> m_text_curve;
    agg::conv_transform<agg::conv_curve<font_manager_type::path_adaptor_type> > m_text_trans;
//    font_renderer m_font_ren;

  public:
    text(const char* text, double size = 10.0, double width = 1.0,
         double hjustif = 0.0, double vjustif = 0.0):
      m_matrix(), m_user_matrix(), m_text_buf(text),
      m_text(), m_trans(m_text, m_user_matrix), m_stroke(m_trans),
      m_x(0.0), m_y(0.0), m_angle(0.0),
      m_text_width(0.0), m_text_height(size),
      m_hjustif(hjustif), m_vjustif(vjustif), m_text_pos(0),
      m_font_eng(), m_font_man(m_font_eng),
      m_text_mtx(), m_text_curve(m_font_man.path_adaptor()), m_text_trans(m_text_curve, m_text_mtx)
      // m_font_ren("LiberationSans-Regular.ttf", 1.4*size)
    {
      m_text.text(m_text_buf.cstr());
      m_text.size(size);
      m_text_width = m_text.text_width();

      m_stroke.width(width);
      m_stroke.line_cap(agg::round_cap);
      m_stroke.line_join(agg::round_join);

      agg::glyph_rendering gren = agg::glyph_ren_outline;
      m_font_eng.load_font("LiberationSans-Regular.ttf", 0, gren);
      m_font_eng.hinting(true);

      double text_height = 1.4 * size;
      m_font_eng.height(text_height);
      m_font_eng.width(text_height * scale_x);
    }

    bool load_glyph()
    {
      m_text_x += m_advance_x;
      m_text_y += m_advance_y;

      if (m_text_pos >= m_text_buf.len())
        return false;

      char c = m_text_buf[m_text_pos];
      const agg::glyph_cache *glyph = m_font_man.glyph(c);
      m_font_man.add_kerning(&m_text_x, &m_text_y);
      m_font_man.init_embedded_adaptors(glyph, 0, 0);

      if(glyph->data_type == agg::glyph_data_outline)
        {
          m_text_mtx.tx = m_text_x / scale_x;
          m_text_mtx.ty = floor(m_text_y + 0.5);
          m_matrix.transform(&m_text_mtx.tx, &m_text_mtx.ty);

          m_advance_x = glyph->advance_x;
          m_advance_y = glyph->advance_y;

          return true;
        }

      return false;
    }

    virtual void rewind(unsigned path_id)
    {
      m_text_x = scale_x * (-m_hjustif * get_text_width());
      m_text_y = -m_vjustif * (1.4 * m_text_height);
      m_advance_x = 0;
      m_advance_y = 0;
      m_text_pos = 0;

      m_text_mtx = m_matrix;
      agg::trans_affine_scaling scale_mtx(1.0 / double(scale_x), 1.0);
      trans_affine_compose (m_text_mtx, scale_mtx);

      load_glyph();
    }

    virtual unsigned vertex(double* x, double* y)
    {
      unsigned cmd = m_text_trans.vertex(x, y);
      if (agg::is_stop(cmd))
        {
          m_text_pos++;
          if (load_glyph())
            {
              return vertex(x, y);
            }
          return agg::path_cmd_stop;
        }
      return cmd;
    }

    void angle(double th) {
      double c = cos(th), s = sin(th);

      m_angle = th;

      m_user_matrix.sx  =  c;
      m_user_matrix.shx = -s;
      m_user_matrix.shy =  s;
      m_user_matrix.sy  =  c;
    }

    double angle() const { return m_angle; };

    const char * get_text() const { return m_text_buf.cstr(); }

    void set_point(double x, double y) {
      m_x = x;
      m_y = y;

      m_user_matrix.tx = m_x;
      m_user_matrix.ty = m_y;
    }

    void hjustif(double hj) { m_hjustif = hj; }
    void vjustif(double vj) { m_vjustif = vj; }

    virtual void apply_transform(const agg::trans_affine& m, double as);
    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2);

    virtual str write_svg(int id, agg::rgba8 c, double h);

    virtual bool use_subpixel() { return true; }

    const vs_text& self() const { return m_text; };
          vs_text& self()       { return m_text; };

  protected:
    double get_text_width()
    {
      unsigned text_length = m_text_buf.len();
      double x = 0, y = 0;

      const char* text = m_text_buf.cstr();
      for (const char* p = text; p < text + text_length; p++)
        {
          const agg::glyph_cache* glyph = m_font_man.glyph(*p);

          if (glyph)
            {
              m_font_man.add_kerning(&x, &y);
              x += glyph->advance_x;
            }
        }

      return x / double(scale_x);
    }
  };
}

#endif
