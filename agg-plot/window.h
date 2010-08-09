#ifndef AGGPLOT_WINDOW_H
#define AGGPLOT_WINDOW_H

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"

extern void window_register           (lua_State *L);

extern int  window_update_unprotected (lua_State *L);
extern int  window_update             (lua_State *L);

__END_DECLS

#endif
