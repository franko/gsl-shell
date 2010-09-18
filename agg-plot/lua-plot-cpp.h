#ifndef AGGPLOT_LUA_PLOT_CPP_H
#define AGGPLOT_LUA_PLOT_CPP_H

#include "lua-plot.h"

extern "C" {
#include "lua.h"
}

#include "plot-auto.h"
#include "resource-manager.h"
#include "drawable.h"

typedef plot<drawable, lua_management> lua_plot;

#endif
