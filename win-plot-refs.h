#ifndef WIN_PLOT_REFS_H
#define WIN_PLOT_REFS_H

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"

extern void window_plot_ref_prepare (lua_State *L);

extern void window_plot_ref_add (lua_State *L, int slot_id, 
				 int window_index, int plot_index);

extern void window_plot_ref_remove (lua_State *L, int slot_id, int window_index);

__END_DECLS

#endif
