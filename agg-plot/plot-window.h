#ifndef PLOT_WINDOW_H
#define PLOT_WINDOW_H

#include <pthread.h>

__BEGIN_DECLS
#include "lua.h"
__END_DECLS

#include "defs.h"

__BEGIN_DECLS

extern void plot_window_register (lua_State *L);

extern pthread_mutex_t window_mutex[1];

#define WINDOW_LOCK() pthread_mutex_lock (window_mutex);
#define WINDOW_UNLOCK() pthread_mutex_unlock (window_mutex);

__END_DECLS

#endif
