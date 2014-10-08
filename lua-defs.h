#ifndef LUA_DEFS_H
#define LUA_DEFS_H

#define INDEX_SET_ABS(L, idx)		      \
  if ((idx) < 0 && (idx) > LUA_REGISTRYINDEX) \
    (idx) = lua_gettop(L) + ((idx)+1)

#define INDEX_SET_ABS_2(L, idx1, idx2) {		\
  int _narg = lua_gettop (L);				\
  if ((idx1) < 0 && (idx1) > LUA_REGISTRYINDEX)		\
    (idx1) = _narg + ((idx1)+1);			\
  if ((idx2) < 0 && (idx2) > LUA_REGISTRYINDEX)		\
    (idx2) = _narg + ((idx2)+1);			\
  }

#endif
