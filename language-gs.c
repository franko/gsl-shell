#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "language.h"
#include "fatal.h"

lua_State *parser_L;

int
language_init() {
    parser_L = lua_open();
      if (unlikely(parser_L == NULL)) {
        fatal_exception("cannot create state: not enough memory");
    }
    luaL_openlibs(parser_L);
    int status = luaL_loadfile(parser_L, "gslang.lua");
    if (status != 0) {
        fatal_exception("unable to load \"gslang.lua\"");
    }
    int load_status = lua_pcall(parser_L, 0, 1, 0);
    if (!lua_isfunction(parser_L, -1)) {
        load_status = LUA_ERRRUN;
    }
    return load_status;
}

int
language_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name)
{
    lua_pushvalue(parser_L, -1);
    lua_pushlstring(parser_L, buff, sz);
    lua_pushstring(parser_L, name);
    int parse_status = lua_pcall(parser_L, 2, 1, 0);
    if (parse_status != 0) {
        const char *msg = lua_tostring(parser_L, -1);
        lua_pushstring(L, msg);
        lua_pop(parser_L, 1);
        return LUA_ERRSYNTAX;
    }
    size_t code_len;
    const char *code = lua_tolstring(parser_L, -1, &code_len);
    int status = luaL_loadbuffer(L, code, code_len, name);
    lua_pop(parser_L, 1);
    return status;
}
