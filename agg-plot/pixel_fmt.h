#ifndef AGGPLOT_PIXEL_FMT_H
#define AGGPLOT_PIXEL_FMT_H

#include "agg-pixfmt-config.h"

#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgb24_lcd.h"

class pixel_gamma_corr {
  typedef gslshell::gamma_type gamma_type;

  gamma_type& m_gamma;
  agg::lcd_distribution_lut m_lut;
public:
  typedef agg::pixfmt_rgb24_gamma<gamma_type> pixfmt_type;
  typedef agg::pixfmt_rgb24_lcd_gamma<gamma_type> pixfmt_lcd_type;

  pixfmt_type pixfmt;
  pixfmt_lcd_type pixfmt_lcd;

  pixel_gamma_corr(agg::rendering_buffer& ren_buf):
    m_gamma(gslshell::gamma), m_lut(1./3., 2./9., 1./9.),
    pixfmt(ren_buf, m_gamma), pixfmt_lcd(ren_buf, m_lut, m_gamma)
  { };

  enum { line_width = 120 };
};

class pixel_simple {
  agg::lcd_distribution_lut m_lut;

public:
  typedef agg::pixfmt_rgb24 pixfmt_type;
  typedef agg::pixfmt_rgb24_lcd pixfmt_lcd_type;

  pixfmt_type pixfmt;
  pixfmt_lcd_type pixfmt_lcd;

  pixel_simple(agg::rendering_buffer& ren_buf):
    m_lut(1./3., 2./9., 1./9.), pixfmt(ren_buf), pixfmt_lcd(ren_buf, m_lut)
  { };

  enum { line_width = 100 };
};

#ifdef DISABLE_GAMMA_CORR
typedef pixel_simple pixel_type;
#else
typedef pixel_gamma_corr pixel_type;
#endif

#endif
