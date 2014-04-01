#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lua.h"
#include "luaconf.h"
#include "lauxlib.h"
#include "lualib.h"
#include "language.h"

#define LANG_INIT_FILENAME "lang/compile.lua"

/* The position in the Lua stack of the loadstring and loadfile functions. */
#define MY_LOADSTRING_INDEX 1
#define MY_LOADFILE_INDEX 2

lua_State *parser_L;

/* This is a Lua function that take one argument, a module name, and
   extracts all the string keys from the global table of the
   corresponding name. The module is probed from an extern Lua State
   stored as an upvalue.
*/
static int
table_keys_xtransfer(lua_State *L) {
    lua_State *xL = lua_touserdata(L, lua_upvalueindex(1));
    const char *mod_name = lua_tostring(L, 1);
    if (xL == NULL || mod_name == NULL) {
        return 0;
    }
    lua_getfield(xL, LUA_GLOBALSINDEX, mod_name);
    if (!lua_istable(xL, -1)) {
        lua_pop(xL, 1);
        return 0;
    }
    lua_newtable(L); /* Create the new table to store the names. */
    lua_pushnil(xL); /* Push the first key (nil) in xL to cann lua_next. */
    while (lua_next(xL, -2) != 0) {
        if (lua_type(xL, -2) == LUA_TSTRING) { /* The key is a string. */
            const char *key = lua_tostring(xL, -2);
            lua_pushstring(L, key);
            lua_pushboolean(L, 1);
            lua_rawset(L, -3); /* Store the string as t[name] = true. */
        }
        lua_pop(xL, 1);
    }
    lua_pop(xL, 1);
    return 1;
}

static void
path_pushstring_sub(lua_State *L, const char *s, size_t len)
{
    if (len == 7 && strncmp(s, "." LUA_DIRSEP "?.lua", 7) == 0) {
        lua_pushstring(L, "lang" LUA_DIRSEP "?.lua");
    } else {
        lua_pushlstring(L, s, len);
    }
}

/* Change package.path by substituting "./?.lua" with "lang/?.lua". */
static int
lang_fix_path(lua_State* L)
{
    int uslots = 1;
    lua_pushstring(L, "package");
    lua_rawget(L, LUA_GLOBALSINDEX);
    if (unlikely(!lua_istable(L, -1))) { goto path_error; }
    lua_pushstring(L, "path");
    lua_rawget(L, -2);
    uslots += 1;
    const char *path = lua_tostring(L, -1);
    if (unlikely(!path)) { goto path_error; }
    lua_pushstring(L, "path");
    lua_pushstring(L, "");
    while (1) {
        const char *c = strchr(path, ';');
        if (!c) break;
        size_t len = c - path;
        path_pushstring_sub(L, path, len);
        lua_pushstring(L, ";");
        lua_concat(L, 3);
        path = c + 1;
    }
    path_pushstring_sub(L, path, strlen(path));
    lua_concat(L, 2);

    lua_rawset(L, -4);
    lua_pop(L, 2);
    return 0;
path_error:
    lua_pop(L, uslots);
    return 1;
}

int
language_init(lua_State *L) {
    parser_L = luaL_newstate();
      if (unlikely(parser_L == NULL)) {
        lua_pushstring(parser_L, "cannot create state: not enough memory");
        return LUA_ERRRUN;
    }
    luaL_openlibs(parser_L);
    lang_fix_path(parser_L);
    int status = luaL_loadfile(parser_L, LANG_INIT_FILENAME);
    if (unlikely(status != 0)) {
        lua_pushstring(parser_L, "unable to load \"" LANG_INIT_FILENAME "\"");
        return LUA_ERRRUN;
    }
    lua_pushlightuserdata(parser_L, (void*) L);
    lua_pushcclosure(parser_L, table_keys_xtransfer, 1);
    int load_status = lua_pcall(parser_L, 1, 2, 0);
    if (!lua_isfunction(parser_L, MY_LOADSTRING_INDEX) ||
        !lua_isfunction(parser_L, MY_LOADFILE_INDEX)) {
        load_status = LUA_ERRRUN;
    }
    return load_status;
}

/* Pop a string from "parser_L" top of the stack and push it in "L" stack. */
static void error_xtransfer(lua_State *L)
{
    const char *msg = lua_tostring(parser_L, -1);
    lua_pushstring(L, msg);
    lua_pop(parser_L, 1);
}

static int loadbuffer_xtransfer(lua_State *L, const char *filename)
{
    size_t code_len;
    const char *code = lua_tolstring(parser_L, -1, &code_len);
    int status = luaL_loadbuffer(L, code, code_len, filename);
    lua_pop(parser_L, 1);
    return status;
}

static int
language_check_error(lua_State *L, const char *filename, int parse_status)
{
    if (parse_status != 0) {
        error_xtransfer(L);
        return LUA_ERRRUN;
    }
    int compile_success = lua_toboolean(parser_L, -2);
    if (!compile_success) {
        error_xtransfer(L);
        lua_pop(parser_L, 1); /* Pop the boolean value. */
        return LUA_ERRSYNTAX;
    }
    int load_status = loadbuffer_xtransfer(L, filename);
    lua_pop(parser_L, 1); /* Pop the boolean value. */
    return load_status;
}

int
language_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name)
{
    lua_pushvalue(parser_L, MY_LOADSTRING_INDEX);
    lua_pushlstring(parser_L, buff, sz);
    lua_pushstring(parser_L, name);
    int parse_status = lua_pcall(parser_L, 2, 2, 0);
    return language_check_error(L, name, parse_status);
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
    int parse_status = lua_pcall(parser_L, 1, 2, 0);
    return language_check_error(L, filename, parse_status);
}
