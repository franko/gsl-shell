#include "agg-pixfmt-config.h"

#ifndef DISABLE_GAMMA_CORR
gslshell::gamma_type gslshell::gamma(1.5);
#endif
agg::lcd_distribution_lut gslshell::subpixel_lut(3./9., 2./9., 1./9.);
