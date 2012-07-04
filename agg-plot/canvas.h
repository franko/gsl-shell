#ifndef AGGPLOT_CANVAS_H
#define AGGPLOT_CANVAS_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "pixel_fmt.h"
#include "sg_object.h"

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_trans_viewport.h"
#include "agg_conv_stroke.h"

template <class Pixel>
class canvas_gen : private Pixel {

  enum { subpixel_scale = 3 };

  typedef agg::renderer_base<typename Pixel::pixfmt_type> renderer_base;
  typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

  renderer_base rb;
  renderer_solid rs;

  agg::renderer_base<typename Pixel::pixfmt_lcd_type> rb_subpixel;
  agg::renderer_scanline_aa_solid<agg::renderer_base<typename Pixel::pixfmt_lcd_type> > rs_subpixel;

  agg::rasterizer_scanline_aa<> ras;
  agg::scanline_u8 sl;

  agg::rgba bg_color;

  double m_width;
  double m_height;

public:
  canvas_gen(agg::rendering_buffer& ren_buf, double width, double height,
             agg::rgba bgcol):
    Pixel(ren_buf), rb(Pixel::pixfmt), rs(rb),
    rb_subpixel(Pixel::pixfmt_lcd), rs_subpixel(rb_subpixel),
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

  template <class VertexSource>
  void draw_solid(VertexSource& vs, agg::rgba8 c)
  {
    this->ras.add_path(vs);
    this->rs.color(c);
    agg::render_scanlines(this->ras, this->sl, this->rs);
  }

  template <class VertexSource>
  void draw_solid_subpixel(VertexSource& vs, agg::rgba8 c)
  {
    agg::trans_affine_scaling subpixel_mtx(double(subpixel_scale), 1.0);
    agg::conv_transform<VertexSource> trans(vs, subpixel_mtx);
    this->ras.add_path(trans);
    this->rs_subpixel.color(c);
    agg::render_scanlines(this->ras, this->sl, this->rs_subpixel);
  }

  void draw(sg_object& vs, agg::rgba8 c)
  {
    if (vs.use_subpixel())
      draw_solid_subpixel(vs, c);
    else
      draw_solid(vs, c);
  }

  void draw_outline(sg_object& vs, agg::rgba8 c)
  {
    agg::conv_stroke<sg_object> line(vs);
    line.width(Pixel::line_width / 100.0);
    line.line_cap(agg::round_cap);

    if (vs.use_subpixel())
      draw_solid_subpixel(line, c);
    else
      draw_solid(line, c);
  }
};

struct virtual_canvas {
  virtual void draw(sg_object& vs, agg::rgba8 c) = 0;
  virtual void draw_outline(sg_object& vs, agg::rgba8 c) = 0;

  virtual void clip_box(const agg::rect_base<int>& clip) = 0;
  virtual void reset_clipping() = 0;

  virtual ~virtual_canvas() { }
};

typedef canvas_gen<pixel_type> canvas;

#endif
