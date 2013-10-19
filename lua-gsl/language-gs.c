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

static char *
read_file_content(lua_State *L, const char *filename, long *plength)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        lua_pushfstring(L, "cannot open %s: %s", filename, strerror(errno));
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    *plength = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = malloc(*plength);
    if (buffer == NULL) {
        lua_pushfstring(L, "not enough memory to open: %s", filename);
        return NULL;
    }
    fread(buffer, 1, *plength, f);
    fclose(f);
    return buffer;
}

static char *
read_stdin(lua_State *L, long *plength)
{
#define inputBufSize 1024
    unsigned char *msg = NULL;
    size_t len = 0;
    unsigned char buffer[inputBufSize];
    size_t br = 0;

    while ((br = fread(buffer, sizeof(buffer), 1, stdin)) > 0) {
        unsigned char *tmp = realloc(msg, len + br);
        if (tmp) {
            msg = tmp;
            memmove(&msg[len], buffer, br);
            len += br;
        } else {
            lua_pushstring(L, "out of memory");
            free(msg);
            return NULL;
        }
    }
    *plength = (long)len;
    return msg;
}

int
language_loadfile(lua_State *L, const char *filename)
{
    char *buffer;
    long length;
    if (filename != NULL) {
        buffer = read_file_content(L, filename, &length);
        if (!buffer) {
            return LUA_ERRFILE;
        }
    } else {
        buffer = read_stdin(L, &length);
    }

    int status = language_loadbuffer(L, buffer, length, filename);
    free(buffer);
    return status;
}

static int
language_lua_loadfile(lua_State* L)
{
    const char *filename;
    if (lua_isnoneornil(L, 1)) {
        filename = NULL;
    } else {
        filename = luaL_checkstring(L, 1);
    }

    int status = language_loadfile(L, filename);
    if (status != 0) {
        return lua_error(L);
    }
    return 1;
}

static int
language_lua_dofile(lua_State* L)
{
    language_lua_loadfile(L);
    int n = lua_gettop(L) - 1;
    lua_call(L, 0, LUA_MULTRET);
    return lua_gettop(L) - n;
}

static int
language_lua_loadstring(lua_State* L)
{
    const char *s = luaL_checkstring(L, 1);
    const char *name;
    if (lua_isnoneornil(L, 2)) {
        name = NULL;
    } else {
        name = luaL_checkstring(L, 2);
    }
    int status = language_loadbuffer(L, s, strlen(s), name);
    if (status != 0) {
        return lua_error(L);
    }
    return 1;
}

static int
language_lua_load(lua_State* L)
{
    if (lua_gettop(L) == 0 || !lua_isfunction(L, 1)) {
        char msg[64];
        sprintf(msg, "function expected, got %s", lua_typename(L, lua_type(L, 1)));
        return luaL_argerror(L, 1, msg);
    }
    str_t buffer;
    str_init(buffer, 1024);
    for (;;) {
        lua_pushvalue(L, 1);
        int status = lua_pcall(L, 0, 1, 0);
        if (status != 0) {
            lua_pushnil(L);
            lua_insert(L, -2);
            return 2;
        }
        if (lua_isnil(L, -1)) {
            break;
        }
        const char *chunk = lua_tostring(L, -1);
        if (!chunk) {
            lua_pushnil(L);
            lua_pushfstring(L, "reader function must return a string");
            return 2;
        }
        str_append_c(buffer, chunk, 0);
        lua_pop(L, 1);
    }
    const char *name;
    if (lua_gettop(L) > 1 && lua_isstring(L, 2)) {
        name = lua_tostring(L, 2);
    } else {
        name = NULL;
    }
    int status = language_loadbuffer(L, CSTR(buffer), STR_LENGTH(buffer), name);
    str_free(buffer);
    if (status != 0) {
        return lua_error(L);
    }
    return 1;
}

static const luaL_Reg language_lib[] = {
  { "load",        language_lua_load },
  { "loadstring",  language_lua_loadstring },
  { "loadfile",    language_lua_loadfile },
  { "dofile",      language_lua_dofile },
  { NULL, NULL }
};

int luaopen_language(lua_State *L)
{
    luaL_register(L, NULL, language_lib);
    return 0;
}
