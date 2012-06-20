#ifndef AGGPLOT_TEXT_H
#define AGGPLOT_TEXT_H

#include "sg_object.h"

#include "agg_rendering_buffer.h"
#include "agg_scanline_p.h"
#include "agg_scanline_bin.h"
#include "agg_renderer_scanline.h"
#include "agg_renderer_primitives.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_gsv_text.h"
#include "agg_trans_affine.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"
#include "agg_pixfmt_rgb.h"
#include "agg_conv_curve.h"
#include "agg_font_freetype.h"

typedef agg::font_engine_freetype_int32 font_engine_type;
typedef agg::font_cache_manager<font_engine_type> font_manager_type;

class font_renderer {
  enum { scale_x = 100, subpixel_scale = 1 };

public:
  font_renderer(const char *font_name, double text_height):
    m_font_eng(), m_font_man(m_font_eng)
  {
    agg::glyph_rendering gren = agg::glyph_ren_outline;
    m_font_eng.load_font(font_name, 0, gren);
    m_font_eng.hinting(true);

    m_font_eng.height(text_height);
    m_font_eng.width(text_height * scale_x * subpixel_scale);
  }

  template <class Rasterizer, class Scanline, class RenSolid>
  void draw_text(Rasterizer& ras, Scanline& sl, RenSolid& ren_solid,
		 const agg::trans_affine& user_matrix,
		 double x, double y, const char* text, int text_length)
  {
    typedef agg::conv_curve<font_manager_type::path_adaptor_type> curve_type;

    x *= subpixel_scale;
    const double start_x = x;

    agg::trans_affine_scaling scale_mtx(1.0 / double(scale_x), 1.0);

    curve_type curves(m_font_man.path_adaptor());
    agg::conv_transform<curve_type> trans1(curves, scale_mtx);
    agg::conv_transform<agg::conv_transform<curve_type> > trans2(trans1, user_matrix);

    for (const char* p = text; p < text + text_length; p++)
      {
	const agg::glyph_cache* glyph = m_font_man.glyph(*p);

	if (!glyph) continue;

	m_font_man.add_kerning(&x, &y);

	m_font_man.init_embedded_adaptors(glyph, 0, 0);
	if(glyph->data_type == agg::glyph_data_outline)
	  {
	    double ty = floor(y + 0.5);

	    scale_mtx.tx = start_x + x / scale_x;
	    scale_mtx.ty = ty;

	    agg::rgba8 color(0, 0, 0);
	    ras.add_path(trans2);
	    ren_solid.color(color);
	    agg::render_scanlines(ras, sl, ren_solid);
	  }

	// increment pen position
	x += glyph->advance_x;
	y += glyph->advance_y;
      }
  }

  double text_width(const char* text, int text_length)
  {
    double x = 0, y = 0;

    for (const char* p = text; p < text + text_length; p++)
      {
	const agg::glyph_cache* glyph = m_font_man.glyph(*p);

	if (glyph)
	  {
	    m_font_man.add_kerning(&x, &y);
	    m_font_man.init_embedded_adaptors(glyph, 0, 0);
	    x += glyph->advance_x;
	  }
      }

    return x / double(scale_x * subpixel_scale);
  }

private:
  font_engine_type m_font_eng;
  font_manager_type m_font_man;
};

namespace draw {

  class text : public sg_object {
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

    font_renderer m_font_ren;

  public:
    text(const char* text, double size = 10.0, double width = 1.0,
	 double hjustif = 0.0, double vjustif = 0.0):
      m_matrix(), m_user_matrix(), m_text_buf(text),
      m_text(), m_trans(m_text, m_user_matrix), m_stroke(m_trans),
      m_x(0.0), m_y(0.0), m_angle(0.0),
      m_text_width(0.0), m_text_height(size),
      m_hjustif(hjustif), m_vjustif(vjustif),
      m_font_ren("georgia.ttf", 1.4*size)
    {
      m_text.text(m_text_buf.cstr());
      m_text.size(size);
      m_text_width = m_text.text_width();

      m_stroke.width(width);
      m_stroke.line_cap(agg::round_cap);
      m_stroke.line_join(agg::round_join);
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

    virtual void rewind(unsigned path_id);
    virtual unsigned vertex(double* x, double* y);
    virtual void apply_transform(const agg::trans_affine& m, double as);
    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2);

    virtual str write_svg(int id, agg::rgba8 c, double h);

    virtual bool render(agg::rendering_buffer& ren_buf,
			agg::rasterizer_scanline_aa<>& ras,
			agg::scanline_p8& sl, agg::rgba8 c);

    const vs_text& self() const { return m_text; };
          vs_text& self()       { return m_text; };
  };
}

#endif
