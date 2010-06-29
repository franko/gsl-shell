#ifndef AGGPLOT_COLORS_H
#define AGGPLOT_COLORS_H

extern "C" {
#include "lua.h"
}

#include "defs.h"
#include "agg_color_rgba.h"

extern agg::rgba8 * rgba8_push_lookup(lua_State *L, const char *color_str);
extern agg::rgba8 * rgba8_push_default(lua_State *L);

#endif
