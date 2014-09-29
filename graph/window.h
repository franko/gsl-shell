#ifndef AGGPLOT_WINDOW_H
#define AGGPLOT_WINDOW_H

#include <pthread.h>

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"

extern void window_register                (lua_State *L, pthread_mutex_t *m);

extern int  window_slot_update             (lua_State *L);
extern int  window_slot_refresh            (lua_State *L);
extern int  window_save_slot_image         (lua_State *L);
extern int  window_restore_slot_image      (lua_State *L);
extern int  window_update                  (lua_State *L);
extern int  window_new                     (lua_State *L);
extern int  window_show                    (lua_State *L);
extern int  window_attach                  (lua_State *L);
extern int  window_close                   (lua_State *L);
extern int  window_close_wait              (lua_State *L);
extern int  window_wait                    (lua_State *L);

extern pthread_mutex_t *graphics_mutex;

__END_DECLS

#endif
