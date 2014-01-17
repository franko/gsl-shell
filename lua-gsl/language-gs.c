#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "language.h"
#include "fatal.h"
#include "luaconf.h"
#include "str.h"

#define LANG_INIT_FILENAME "lang-init.lua"

/* The position in the Lua stack of the loadstring and loadfile functions. */
#define MY_LOADSTRING_INDEX 1
#define MY_LOADFILE_INDEX 2

lua_State *parser_L;

int
language_init() {
    parser_L = lua_open();
      if (unlikely(parser_L == NULL)) {
        fatal_exception("cannot create state: not enough memory");
    }
    luaL_openlibs(parser_L);
    int status = luaL_loadfile(parser_L, LANG_INIT_FILENAME);
    if (status != 0) {
        fatal_exception("unable to load \"" LANG_INIT_FILENAME "\"");
    }
    int load_status = lua_pcall(parser_L, 0, 2, 0);
    if (!lua_isfunction(parser_L, MY_LOADSTRING_INDEX) ||
        !lua_isfunction(parser_L, MY_LOADFILE_INDEX)) {
        load_status = LUA_ERRRUN;
    }
    return load_status;
}

static int error_xtransfer(lua_State *L)
{
    const char *msg = lua_tostring(parser_L, -1);
    lua_pushstring(L, msg);
    lua_pop(parser_L, 1);
    return LUA_ERRSYNTAX;
}

static int loadbuffer_xtransfer(lua_State *L, const char *filename)
{
    size_t code_len;
    const char *code = lua_tolstring(parser_L, -1, &code_len);
    int status = luaL_loadbuffer(L, code, code_len, filename);
    lua_pop(parser_L, 1);
    return status;
}

int
language_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name)
{
    lua_pushvalue(parser_L, MY_LOADSTRING_INDEX);
    lua_pushlstring(parser_L, buff, sz);
    lua_pushstring(parser_L, name);
    int parse_status = lua_pcall(parser_L, 2, 1, 0);
    if (parse_status != 0) {
        return error_xtransfer(L);
    }
    return loadbuffer_xtransfer(L, name);
}

static void l_message(const char *pname, const char *msg)
{
    if (pname) fprintf(stderr, "%s: ", pname);
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
}

int language_report(lua_State *_L, int status)
{
    lua_State *L = parser_L;
    if (status && !lua_isnil(L, -1)) {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error object is not a string)";
        l_message("<GSL Shell parser>", msg);
        lua_pop(L, 1);
    }
    return status;
}

int
language_loadfile(lua_State *L, const char *filename)
{
    lua_pushvalue(parser_L, MY_LOADFILE_INDEX);
    lua_pushstring(parser_L, filename);
    int parse_status = lua_pcall(parser_L, 1, 1, 0);
    if (parse_status != 0) {
        return error_xtransfer(L);
    }
    return loadbuffer_xtransfer(L, filename);
}
