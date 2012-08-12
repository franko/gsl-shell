#ifndef FOXGUI_FOX_WINDOW_H
#define FOXGUI_FOX_WINDOW_H

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"

extern void fox_window_register (lua_State *L);

extern int fox_window_new                 (lua_State *L);
extern int fox_window_attach              (lua_State *L);
extern int fox_window_close               (lua_State *L);
extern int fox_window_slot_refresh        (lua_State *L);
extern int fox_window_slot_update         (lua_State *L);
extern int fox_window_save_slot_image     (lua_State *L);
extern int fox_window_restore_slot_image  (lua_State *L);

__END_DECLS

#endif
