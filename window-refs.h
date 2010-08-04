#ifndef WINDOW_REFS_H
#define WINDOW_REFS_H

#include "defs.h"

__BEGIN_DECLS

#include <lua.h>

extern void   window_ref_prepare   (lua_State *L);
extern size_t window_ref_add       (lua_State *L, int index);
extern void   window_ref_remove    (lua_State *L, int id);

__END_DECLS

#endif
