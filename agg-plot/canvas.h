#ifndef AGGPLOT_CANVAS_H
#define AGGPLOT_CANVAS_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_pixfmt_rgb.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "agg_trans_viewport.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_renderer_outline_aa.h"
#include "agg_gamma_lut.h"

#include "utils.h"

class canvas {
  typedef agg::gamma_lut<agg::int8u, agg::int16u, 8, 12> gamma_type;
  typedef agg::pixfmt_bgr24_gamma<gamma_type> pixel_fmt;
  typedef agg::renderer_base<pixel_fmt> renderer_base;
  typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;
  typedef agg::renderer_outline_aa<renderer_base> renderer_oaa;
  typedef agg::rasterizer_outline_aa<renderer_oaa> rasterizer_outline_aa;

  gamma_type m_gamma;
  pixel_fmt pixf;
  renderer_base rb;
  renderer_solid rs;

  agg::line_profile_aa prof;
  renderer_oaa ren_oaa;
  rasterizer_outline_aa ras_oaa;

  agg::rasterizer_scanline_aa<> ras;
  agg::scanline_p8 sl;

  agg::rgba bg_color;

  agg::trans_affine mtx;

  double m_width;
  double m_height;

public:
  canvas(agg::rendering_buffer& ren_buf, double width, double height, 
	 agg::rgba bgcol): 
    m_gamma(2.2), pixf(ren_buf, m_gamma), rb(pixf), rs(rb),
    prof(), ren_oaa(rb, prof), ras_oaa(ren_oaa),
    ras(), sl(), bg_color(bgcol),
    m_width(width), m_height(height)
  {
    mtx.scale(width, height);
    prof.width(1.5);
  };

  double width() const { return m_width; };
  double height() const { return m_height; };

  void clear() { rb.clear(bg_color); };

  const agg::trans_affine& trans_matrix() const { return mtx; };
  void scale(agg::trans_affine& m) const { trans_affine_compose (m, mtx); };

  template<class VertexSource>
  void draw(VertexSource& vs, agg::rgba8 c)
  {
    ras.add_path(vs);
    rs.color(c);
    agg::render_scanlines(ras, sl, rs);
  };

  template<class VertexSource>
  void draw_outline(VertexSource& vs, agg::rgba8 c)
  {
    ren_oaa.color(c);
    ras_oaa.add_path(vs);
  };
};

#endif
