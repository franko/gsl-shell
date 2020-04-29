#include <string.h>
#include <lua.h>
#include <lauxlib.h>

#include "core/matrix_bc_loader.h"

#include "matrix/cblas.h"
#include "matrix/matrix-core.h"
#include "matrix/matrix-display.h"

struct bcpair {
    const char *name;
    const char *bc;
    size_t size;
};

/* TODO: rename matrix/matrix-core to matrix/core and use the
   luajit option -n to give explicitly the module name.
   It is important to avoid conflict with bytecode module names
   imported from language so a common prefix is needed. */
static struct bcpair bcmodule[] = {
    {"cblas", luaJIT_BC_cblas, luaJIT_BC_cblas_SIZE},
    {"matrix-core", luaJIT_BC_matrix_core, luaJIT_BC_matrix_core_SIZE},
    {"matrix-display", luaJIT_BC_matrix_display, luaJIT_BC_matrix_display_SIZE},
    {0, 0, 0}
};

/* Load into package.preload matrix modules using embedded bytecode. */
void matrix_bc_preload(lua_State *L)
{
    struct bcpair *i;
    lua_getfield(L, LUA_GLOBALSINDEX, "package");
    lua_getfield(L, -1, "preload");
    for (i = bcmodule; i->name; i++) {
        lua_pushstring(L, "matrix.");
        lua_pushstring(L, i->name);
        lua_concat(L, 2);
        luaL_loadbuffer(L, i->bc, i->size, lua_tostring(L, -1));
        lua_rawset(L, -3);
    }
    lua_pop(L, 2);
}
