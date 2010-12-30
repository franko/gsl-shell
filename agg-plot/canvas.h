#ifndef AGGPLOT_CANVAS_H
#define AGGPLOT_CANVAS_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_pixfmt_rgb.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "agg_trans_viewport.h"
#include "agg_conv_stroke.h"

#include "agg_gamma_lut.h"

class pixel_gamma_corr {
  typedef agg::gamma_lut<agg::int8u, agg::int16u, 8, 12> gamma_type;
  typedef agg::pixfmt_bgr24_gamma<gamma_type> pixel_fmt;

  gamma_type m_gamma;

public:
  typedef pixel_fmt fmt;

  pixel_gamma_corr(agg::rendering_buffer& ren_buf): 
    m_gamma(2.2), pixfmt(ren_buf, m_gamma) 
  { };

  pixel_fmt pixfmt;
};

struct pixel_simple {
  agg::pixfmt_bgr24 pixfmt;

  typedef agg::pixfmt_bgr24 fmt;

  pixel_simple(agg::rendering_buffer& ren_buf): pixfmt(ren_buf) { };
};

template <class pixel, int linewidth>
class canvas_gen : private pixel {
  typedef agg::renderer_base<typename pixel::fmt> renderer_base;
  typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

  renderer_base rb;
  renderer_solid rs;

  agg::rasterizer_scanline_aa<> ras;
  agg::scanline_p8 sl;

  agg::rgba bg_color;

  double m_width;
  double m_height;

public:
  canvas_gen(agg::rendering_buffer& ren_buf, double width, double height, 
	     agg::rgba bgcol): 
    pixel(ren_buf), rb(pixel::pixfmt), rs(rb),
    ras(), sl(), bg_color(bgcol),
    m_width(width), m_height(height)
  {
  };

  double width()  const { return m_width; };
  double height() const { return m_height; };

  void clear() { rb.clear(bg_color); };

  void clear_box(const agg::rect_base<int>& r)
  {
    for (int y = r.y1; y < r.y2; y++)
      this->rb.copy_hline (r.x1, y, r.x2, bg_color);
  };

  void clip_box(const agg::rect_base<int>& clip)
  {
    this->rb.clip_box_naked(clip.x1, clip.y1, clip.x2, clip.y2);
  };

  void reset_clipping() { this->rb.reset_clipping(true); };

  template<class VertexSource>
  void draw(VertexSource& vs, agg::rgba8 c)
  {
    this->ras.add_path(vs);
    this->rs.color(c);
    agg::render_scanlines(this->ras, this->sl, this->rs);
  };

  template<class VertexSource>
  void draw_outline(VertexSource& vs, agg::rgba8 c)
  {
    agg::conv_stroke<VertexSource> line(vs);
    line.width(linewidth / 10.0L);
    line.line_cap(agg::round_cap);

    this->ras.add_path(line);
    this->rs.color(c);
    agg::render_scanlines(this->ras, this->sl, this->rs);
  };
};

#ifdef DISABLE_GAMMA_CORR
typedef canvas_gen<pixel_simple, 10> canvas;
#else
typedef canvas_gen<pixel_gamma_corr, 15> canvas;
#endif

#endif
