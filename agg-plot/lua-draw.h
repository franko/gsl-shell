#ifndef LUA_DRAW_H
#define LUA_DRAW_H


#include "defs.h"

#include <pthread.h>

__BEGIN_DECLS
#include "lua.h"
__END_DECLS

#ifdef __cplusplus

#include "vertex-source.h"
#include "drawables.h"
#include "trans.h"

extern int agg_text_new       (lua_State *L);
extern int agg_path_new       (lua_State *L);
extern int agg_rgb_new        (lua_State *L);
extern int agg_rgba_new       (lua_State *L);

extern vertex_source*  check_agg_obj   (lua_State *L, int index);
extern my::path*       check_agg_path  (lua_State *L, int index);
extern my::text*       check_agg_text  (lua_State *L, int index);
extern agg::rgba8*     check_agg_rgba8 (lua_State *L, int index);

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
