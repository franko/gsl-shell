#ifndef LUA_GRAPH_H
#define LUA_GRAPH_H

#include "defs.h"

__BEGIN_DECLS

#include <lua.h>

extern void graph_close_windows (lua_State *L);
extern void register_graph (lua_State *L);
extern void gsl_shell_close_with_graph (struct gsl_shell_state* gs);

__END_DECLS

#endif
