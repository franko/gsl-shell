#ifndef LUA_GSL_H
#define LUA_GSL_H

#include <pthread.h>

#include "defs.h"

__BEGIN_DECLS
#include <lua.h>

extern pthread_mutex_t gsl_shell_mutex[1];
extern pthread_mutex_t gsl_shell_shutdown_mutex[1];
extern volatile int gsl_shell_shutting_down;

extern void gsl_shell_init ();
extern void gsl_shell_close ();

extern int luaopen_gsl (lua_State *L);

__END_DECLS

#define GSL_SHELL_LOCK() pthread_mutex_lock (gsl_shell_mutex)
#define GSL_SHELL_UNLOCK() pthread_mutex_unlock (gsl_shell_mutex)

#endif
