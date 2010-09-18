#ifndef OBJECT_REFS_H
#define OBJECT_REFS_H

#include "defs.h"

__BEGIN_DECLS

#include <lua.h>

extern void object_ref_prepare (lua_State *L);
extern void object_ref_add     (lua_State *L, int key_index, int val_index);
extern void object_ref_remove  (lua_State *L, int key_index, int val_index);

__END_DECLS

#endif
