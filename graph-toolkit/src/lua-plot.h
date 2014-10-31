#ifndef AGGPLOT_LUA_PLOT_H
#define AGGPLOT_LUA_PLOT_H

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"

extern void plot_register (lua_State *L);

extern void plot_lua_add_ref (lua_State* L, int plot_index, int ref_index);

__END_DECLS

#endif
