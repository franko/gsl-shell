#ifndef AGG_PIXFMT_CONFIG_H
#define AGG_PIXFMT_CONFIG_H

#include "platform/agg_platform_support.h"
#include "agg_gamma_lut.h"
#include "agg_pixfmt_rgb24_lcd.h"

namespace gslshell
{
typedef agg::gamma_lut<agg::int8u, agg::int16u, 8, 12> gamma_type;

const agg::pix_format_e pixel_format = agg::pix_format_rgb24;
const bool flip_y = true;

extern agg::pix_format_e  sys_pixel_format;
extern unsigned      sys_bpp;

extern gamma_type gamma;
extern agg::lcd_distribution_lut subpixel_lut;
}

#endif
