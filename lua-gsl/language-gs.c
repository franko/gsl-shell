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

int
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

int
language_lua_dofile(lua_State* L)
{
    language_lua_loadfile(L);
    int n = lua_gettop(L);
    lua_call(L, 0, LUA_MULTRET);
    return lua_gettop(L) - n;
}
/* ------------------------------------------------------------------------ */

static int readable(const char *filename)
{
  FILE *f = fopen(filename, "r");  /* try to open file */
  if (f == NULL) return 0;  /* open failed */
  fclose(f);
  return 1;
}

static const char *pushnexttemplate(lua_State *L, const char *path)
{
  const char *l;
  while (*path == *LUA_PATHSEP) path++;  /* skip separators */
  if (*path == '\0') return NULL;  /* no more templates */
  l = strchr(path, *LUA_PATHSEP);  /* find next separator */
  if (l == NULL) l = path + strlen(path);
  lua_pushlstring(L, path, (size_t)(l - path));  /* template */
  return l;
}

static const char *searchpath (lua_State *L, const char *name,
                   const char *path, const char *sep,
                   const char *dirsep)
{
  luaL_Buffer msg;  /* to build error message */
  luaL_buffinit(L, &msg);
  if (*sep != '\0')  /* non-empty separator? */
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  while ((path = pushnexttemplate(L, path)) != NULL) {
    const char *filename = luaL_gsub(L, lua_tostring(L, -1),
                     LUA_PATH_MARK, name);
    lua_remove(L, -2);  /* remove path template */
    if (readable(filename))  /* does file exist and is readable? */
      return filename;  /* return that file name */
    lua_pushfstring(L, "\n\tno file " LUA_QS, filename);
    lua_remove(L, -2);  /* remove file name */
    luaL_addvalue(&msg);  /* concatenate error msg. entry */
  }
  luaL_pushresult(&msg);  /* create error message */
  return NULL;  /* not found */
}

static const char *findfile(lua_State *L, const char *name,
                const char *pname)
{
  const char *path;
  lua_getfield(L, LUA_GLOBALSINDEX, "package");
  lua_getfield(L, -1, pname);
  path = lua_tostring(L, -1);
  if (path == NULL)
    luaL_error(L, LUA_QL("package.%s") " must be a string", pname);
  return searchpath(L, name, path, ".", LUA_DIRSEP);
}

static void loaderror(lua_State *L, const char *filename)
{
  luaL_error(L, "error loading module " LUA_QS " from file " LUA_QS ":\n\t%s",
         lua_tostring(L, 1), filename, lua_tostring(L, -1));
}

int language_lua_package_loader(lua_State *L)
{
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path");
  if (filename == NULL) return 1;  /* library not found in this path */
  if (language_loadfile(L, filename) != 0)
    loaderror(L, filename);
  return 1;  /* library loaded successfully */
}
