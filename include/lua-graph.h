#ifndef LUA_GRAPH_H
#define LUA_GRAPH_H

#include <pthread.h>

#include "defs.h"

__BEGIN_DECLS

#include <lua.h>

extern void graph_close_windows (lua_State *L);
extern void register_graph (lua_State *L);
extern void gsl_shell_close_with_graph (struct gsl_shell_state* gs, int send_close_req);

extern int initialize_fonts(lua_State* L);

extern pthread_mutex_t agg_mutex[1];

#define AGG_LOCK() pthread_mutex_lock (agg_mutex);
#define AGG_UNLOCK() pthread_mutex_unlock (agg_mutex);

__END_DECLS

#endif
