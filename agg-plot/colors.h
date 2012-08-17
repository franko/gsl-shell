#ifndef AGGPLOT_COLORS_H
#define AGGPLOT_COLORS_H

extern "C" {
#include "lua.h"
}

#include "defs.h"
#include "agg_color_rgba.h"

extern agg::rgba8 color_arg_lookup   (lua_State *L, int index);

namespace colors {

  extern agg::rgba8 white;
  extern agg::rgba8 black;

  extern agg::rgba8 cdefault;

  extern agg::rgba8 background;
  extern agg::rgba8 foreground;
  extern agg::rgba8 foreground_b;
};

#endif
