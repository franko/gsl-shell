#ifndef GRAPHICS_HOOKS_H
#define GRAPHICS_HOOKS_H

#include <lua.h>

#include "defs.h"

__BEGIN_DECLS

struct __graphics_lib {
	void (*init)(lua_State *L);
	void (*close_windows)(lua_State *L);
	void (*wait_windows)(lua_State *L);
};

typedef struct __graphics_lib graphics_lib;

extern const graphics_lib graphics[1];

__END_DECLS

#endif
