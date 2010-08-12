#ifndef CANVAS_WINDOW_H
#define CANVAS_WINDOW_H

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"

extern void canvas_window_register        (lua_State *L);
extern int  canvas_window_close_protected (lua_State *L);

__END_DECLS

#endif
