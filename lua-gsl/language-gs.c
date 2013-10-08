#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
