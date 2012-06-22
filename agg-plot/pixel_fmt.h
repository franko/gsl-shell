#ifndef AGGPLOT_PIXEL_FMT_H
#define AGGPLOT_PIXEL_FMT_H

#include "agg_pixfmt_rgb.h"
#include "agg_gamma_lut.h"

class pixel_gamma_corr {
  typedef agg::gamma_lut<agg::int8u, agg::int16u, 8, 12> gamma_type;
  typedef agg::pixfmt_bgr24_gamma<gamma_type> pixel_fmt;

  gamma_type m_gamma;
public:
  typedef pixel_fmt fmt;

  pixel_fmt pixfmt;

  pixel_gamma_corr(agg::rendering_buffer& ren_buf):
    m_gamma(2.2), pixfmt(ren_buf, m_gamma)
  { };

  enum { line_width = 150 };
};

struct pixel_simple {
  agg::pixfmt_bgr24 pixfmt;

  typedef agg::pixfmt_bgr24 fmt;

  pixel_simple(agg::rendering_buffer& ren_buf): pixfmt(ren_buf) { };

  enum { line_width = 100 };
};

#ifdef DISABLE_GAMMA_CORR
typedef pixel_simple pixel_type;
#else
typedef pixel_gamma_corr pixel_type;
#endif

#endif
