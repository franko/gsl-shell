#ifndef AGGPLOT_COLORS_H
#define AGGPLOT_COLORS_H

#include "defs.h"

__BEGIN_DECLS

struct color {
  unsigned int r, g, b, a;
};

extern void color_lookup        (struct color *c, const char *color_str);
extern void set_color_default   (struct color *c);

__END_DECLS

#endif
