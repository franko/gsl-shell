#ifndef AGGPLOT_PIXEL_FMT_H
#define AGGPLOT_PIXEL_FMT_H

#include "agg-pixfmt-config.h"

#include "agg_pixfmt_rgb.h"

typedef gslshell::gamma_type gamma_type;

struct pixel_gamma : public agg::pixfmt_rgb24_gamma<gamma_type>
{
    pixel_gamma(agg::rendering_buffer& ren_buf):
    agg::pixfmt_rgb24_gamma<gamma_type>(ren_buf, gslshell::gamma)
    { }
};

struct pixel_gamma_lcd : public agg::pixfmt_rgb24_lcd_gamma<gamma_type>
{
    pixel_gamma_lcd(agg::rendering_buffer& ren_buf):
    agg::pixfmt_rgb24_lcd_gamma<gamma_type>(ren_buf, gslshell::subpixel_lut, gslshell::gamma)
    { }
};

struct pixel_simple : public agg::pixfmt_rgb24
{
    pixel_simple(agg::rendering_buffer& ren_buf):
    agg::pixfmt_rgb24(ren_buf)
    { }

  enum { line_width = 100 };
};

#ifdef DISABLE_GAMMA_CORR
typedef pixel_simple pixel_type;
#else
typedef pixel_gamma pixel_type;
#endif

#endif
