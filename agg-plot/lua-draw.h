#ifndef LUA_DRAW_H
#define LUA_DRAW_H


#include "defs.h"

#include <pthread.h>

__BEGIN_DECLS
#include "lua.h"
__END_DECLS

#ifdef __cplusplus

#include "agg_color_rgba.h"

#include "sg_object.h"
#include "path.h"

extern int agg_text_new       (lua_State *L);
extern int agg_path_new       (lua_State *L);

extern draw::path*     check_agg_path  (lua_State *L, int index);

#endif

__BEGIN_DECLS

extern pthread_mutex_t agg_mutex[1];

#define AGG_LOCK() pthread_mutex_lock (agg_mutex);
#define AGG_UNLOCK() pthread_mutex_unlock (agg_mutex);

#define AGG_PROTECT(op) {				\
    pthread_mutex_lock (agg_mutex);			\
    op;							\
    pthread_mutex_unlock (agg_mutex);			\
  }

extern void draw_register (lua_State *L);

__END_DECLS

#endif
