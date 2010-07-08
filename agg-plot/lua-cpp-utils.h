#ifndef LUA_CPP_UTILS_H
#define LUA_CPP_UTILS_H

#include <new>

#include "defs.h"
__BEGIN_DECLS
#include "lua.h"
__END_DECLS

#include "gs-types.h"

inline void* operator new(size_t nbytes, lua_State *L, enum gs_type_e tp)
{
  void* p = lua_newuserdata(L, nbytes);
  gs_set_metatable (L, tp);
  return p;
}

#endif
