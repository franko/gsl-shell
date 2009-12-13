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

#include "utils.h"

class canvas {
  typedef agg::pixfmt_bgr24 pixel_fmt;
  typedef agg::renderer_base<pixel_fmt> renderer_base;
  typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

  pixel_fmt *pixf;
  renderer_base *rb;
  renderer_solid *rs;

  agg::rasterizer_scanline_aa<> ras;
  agg::scanline_p8 sl;

  agg::rgba bg_color;

  agg::trans_affine mtx;

  double m_width;
  double m_height;

public:
  canvas(agg::rendering_buffer& ren_buf, double width, double height, 
	 agg::rgba bgcol): 
    ras(), sl(), bg_color(bgcol), mtx(), m_width(width), m_height(height)
  {
    pixf = new pixel_fmt(ren_buf);
    rb   = new renderer_base(*pixf);
    rs   = new renderer_solid(*rb);

    mtx.scale(width, height);
  };

  ~canvas() {
    free (rs);
    free (rb);
    free (pixf);
  };

  double width() const { return m_width; };
  double height() const { return m_height; };

  void clear() { rb->clear(bg_color); };

  const agg::trans_affine& trans_matrix() const { return mtx; };
  void scale(agg::trans_affine& m) const { trans_affine_compose (m, mtx); };

  template<class VertexSource>
  void draw(VertexSource& vs, agg::rgba8 c)
  {
    if (rs == NULL)
      return;

    ras.add_path(vs);
    rs->color(c);
    agg::render_scanlines(ras, sl, *rs);
  };
};

#endif
