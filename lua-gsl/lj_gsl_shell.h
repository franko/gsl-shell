#ifndef LJ_GSL_SHELL_H
#define LJ_GSL_SHELL_H

#include "defs.h"

#include <pthread.h>
#include "lua.h"

__BEGIN_DECLS

struct gsl_shell_app;

struct lj_gsl_shell_state {
  pthread_mutex_t agg_mutex;
  struct gsl_shell_app *app;
};

extern void lj_gsl_shell_init(lua_State *L, struct gsl_shell_app *app);
extern struct lj_gsl_shell_state* lj_gsl_shell_state_get(lua_State *L);

__END_DECLS

#endif
