#ifndef AGGPLOT_WINDOW_HOOKS_H
#define AGGPLOT_WINDOW_HOOKS_H

#include <pthread.h>

#include "defs.h"

__BEGIN_DECLS

#include "lua.h"

struct window_hooks {
    int (*create)(lua_State* L);
    int (*show)(lua_State* L);
    int (*attach)(lua_State* L);
    int (*update)(lua_State* L);
    int (*refresh)(lua_State* L);
    int (*close)(lua_State* L);
    int (*wait)(lua_State* L);
    int (*save_image)(lua_State* L);
    int (*restore_image)(lua_State* L);
};

extern struct window_hooks *app_window_hooks;

__END_DECLS

#endif
