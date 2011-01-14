#ifndef OBJECT_INDEX_H
#define OBJECT_INDEX_H

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"

extern void   object_index_prepare   (lua_State *L);
extern int    object_index_add       (lua_State *L, int index);
extern void   object_index_get       (lua_State *L, int id);
extern void   object_index_remove    (lua_State *L, int id);
extern void   object_index_apply_all (lua_State *L, lua_CFunction f);
extern int    object_index_count     (lua_State *L);

__END_DECLS

#endif
