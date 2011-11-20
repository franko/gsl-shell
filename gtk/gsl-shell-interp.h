#ifndef GSL_SHELL_INTERP_H
#define GSL_SHELL_INTERP_H

#include "lua.h"

extern int         gsl_shell_exec  (lua_State *L, const char *line);
extern lua_State * gsl_shell_init  ();
extern void        gsl_shell_close (lua_State *L);

#endif
