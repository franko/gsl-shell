#ifndef WINDOW_REGISTRY_H
#define WINDOW_REGISTRY_H

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"
#include "lauxlib.h"

extern void   window_registry_prepare   (lua_State *L);

extern int    window_index_add       (lua_State *L, int index);
extern void   window_index_get       (lua_State *L, int id);
extern void   window_index_remove    (lua_State *L, int id);
extern void   window_index_apply_all (lua_State *L, lua_CFunction f);
extern int    window_index_count     (lua_State *L);
extern void   window_index_remove_fun(lua_State *L);

extern void window_refs_add          (lua_State *L, int index, int key_index,
                                      int value_index);
extern void window_refs_remove       (lua_State *L, int index, int key_index);
extern void window_refs_lookup_apply (lua_State *L, int value_index,
                                      lua_CFunction func);

__END_DECLS

#endif
