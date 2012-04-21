
#include "lj_gsl_shell.h"

void
lj_gsl_shell_init(lua_State *L, struct gsl_shell_app *app)
{
  struct lj_gsl_shell_state* gss = lj_gsl_shell_state_get(L);
  pthread_mutex_init(&gss->agg_mutex, NULL);
  gss->app = app;
}
