#ifndef AGGPLOT_FONT_RENDERER_H
#define AGGPLOT_FONT_RENDERER_H

#include "agg_renderer_scanline.h"
#include "agg_conv_transform.h"
#include "agg_font_freetype.h"
#include "agg_conv_curve.h"


class font_renderer {
  typedef agg::font_engine_freetype_int32 font_engine_type;
  typedef agg::font_cache_manager<font_engine_type> font_manager_type;

  enum { scale_x = 100, subpixel_scale = 3 };

public:
  font_renderer(const char *font_name, double text_height):
    m_font_eng(), m_font_man(m_font_eng)
  {
    agg::glyph_rendering gren = agg::glyph_ren_outline;
    m_font_eng.load_font(font_name, 0, gren);
    m_font_eng.hinting(true);

    m_font_eng.height(text_height);
    m_font_eng.width(text_height * scale_x);
  }

  template <class Rasterizer, class Scanline, class RenSolid>
  void draw_text(Rasterizer& _ras, Scanline& _sl, RenSolid& ren_solid,
                 const agg::trans_affine& user_matrix,
                 double x, double y, const char* text, int text_length,
                 agg::rgba8 color)
  {
    typedef agg::conv_curve<font_manager_type::path_adaptor_type> curve_type;

    agg::scanline_u8 sl;
    agg::rasterizer_scanline_aa<> ras;

    agg::trans_affine mtx = user_matrix;
    agg::trans_affine_scaling scale_mtx(1.0 / double(scale_x), 1.0);
    trans_affine_compose (mtx, scale_mtx);

    curve_type curves(m_font_man.path_adaptor());
    agg::conv_transform<curve_type> trans(curves, mtx);

    agg::trans_affine_scaling subpixel_mtx(subpixel_scale, 1.0);
    agg::conv_transform<agg::conv_transform<curve_type> > subpixel_trans(trans, subpixel_mtx);

    const double start_x = x;

    for (const char* p = text; p < text + text_length; p++)
      {
        const agg::glyph_cache* glyph = m_font_man.glyph(*p);

        if (!glyph) continue;

        m_font_man.add_kerning(&x, &y);

        m_font_man.init_embedded_adaptors(glyph, 0, 0);
        if(glyph->data_type == agg::glyph_data_outline)
          {
            mtx.tx = start_x + x / scale_x;
            mtx.ty = floor(y + 0.5);

            user_matrix.transform(&mtx.tx, &mtx.ty);

            ras.add_path(subpixel_trans);
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
            /* We suppose that the embedded adaptors doesn't play to
               determine the text width. */
            /* m_font_man.init_embedded_adaptors(glyph, 0, 0); */
            x += glyph->advance_x;
          }
      }

      return x / double(scale_x);
  }

private:
  font_engine_type m_font_eng;
  font_manager_type m_font_man;
};

#endif
