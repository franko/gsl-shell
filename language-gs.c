#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "language.h"
#include "fatal.h"

struct gs_language {
    lua_State *L;
};

void *
language_init() {
    struct gs_language *gs = malloc(sizeof(struct gs_language));
    if (unlikely(gs == NULL)) {
        fatal_exception("virtual memory exhausted");
    }
    gs->L = lua_open();
      if (unlikely(gs->L == NULL)) {
        fatal_exception("cannot create state: not enough memory");
    }
    luaL_openlibs(gs->L);
    int status = luaL_loadfile(gs->L, "gslang.lua");
    if (status != 0) {
        fatal_exception("unable to load \"gslang.lua\"");
    }
    lua_pcall(gs->L, 0, 1, 0);
    return (void *) gs;
}

int
language_loadbuffer(void *_gs, lua_State *L, const char *buff, size_t sz, const char *name)
{
    struct gs_language *gs = _gs;
    lua_pushvalue(gs->L, -1);
    lua_pushlstring(gs->L, buff, sz);
    lua_pushstring(gs->L, name);
    int parse_status = lua_pcall(gs->L, 2, 1, 0);
    if (parse_status != 0) {
        return parse_status;
    }
    size_t code_len;
    const char *code = lua_tolstring(gs->L, -1, &code_len);
    int status = luaL_loadbuffer(L, code, code_len, name);
    lua_pop(gs->L, 1);
    return status;
}
