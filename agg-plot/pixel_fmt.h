#ifndef AGGPLOT_PIXEL_FMT_H
#define AGGPLOT_PIXEL_FMT_H

#include "agg-pixfmt-config.h"

#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgb24_lcd.h"

class pixel_gamma_corr {
  typedef gslshell::gamma_type gamma_type;
  typedef agg::pixfmt_rgb24_gamma<gamma_type> pixel_fmt;

  gamma_type& m_gamma;
  agg::lcd_distribution_lut m_lut;
public:
  typedef pixel_fmt fmt;
  typedef agg::pixfmt_rgb24_lcd<gamma_type> lcd_fmt;

  fmt pixfmt;
  lcd_fmt pixfmt_lcd;

  pixel_gamma_corr(agg::rendering_buffer& ren_buf):
    m_gamma(gslshell::gamma), m_lut(1./3., 2./9., 1./9.),
    pixfmt(ren_buf, m_gamma), pixfmt_lcd(ren_buf, m_lut, m_gamma)
  { };

  enum { line_width = 120 };
};

struct pixel_simple {
  agg::pixfmt_rgb24 pixfmt;

  typedef agg::pixfmt_rgb24 fmt;

  pixel_simple(agg::rendering_buffer& ren_buf): pixfmt(ren_buf) { };

  enum { line_width = 100 };
};

#ifdef DISABLE_GAMMA_CORR
typedef pixel_simple pixel_type;
#else
typedef pixel_gamma_corr pixel_type;
#endif

#endif
