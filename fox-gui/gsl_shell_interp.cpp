#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

#define luajit_c

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"
}
// #include "lua-gsl.h"
// #include "gsl-shell.h"

#include  "gsl_shell_interp.h"

/*
static void gsl_shell_openlibs(lua_State *L)
{
  luaopen_gsl (L);
}
*/

static int
report(lua_State *L, int status)
{
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    fprintf(stderr, "%s\n", msg);
    lua_pop(L, 1);
  }
  return status;
}

static int pinit(lua_State *L)
{
  LUAJIT_VERSION_SYM();  /* linker-enforced version check */
  lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
  luaL_openlibs(L);  /* open libraries */
  //  gsl_shell_openlibs(L);
  lua_gc(L, LUA_GCRESTART, -1);
  //  dolibrary (L, "gslext");
  return 0;
}

gsl_shell::~gsl_shell()
{
  if (m_lua_state)
    lua_close(m_lua_state);
}

void gsl_shell::init()
{
  lua_State *L;
  int status;

  L = lua_open();  /* create state */

  if (L == NULL)
    throw std::runtime_error("cannot create state: not enough memory");

  status = lua_cpcall(L, pinit, NULL);

  if (report(L, status))
    {
      lua_close(L);
      throw std::runtime_error("cannot initialize Lua state");
    }

  m_lua_state = L;
}

int gsl_shell::exec (const char *line)
{
  m_interp.lock();

  lua_State* L = m_lua_state;
  int status = luaL_loadbuffer(L, line, strlen(line), "=<user input>");
  if (status == 0)
    {
      status = lua_pcall(L, 0, 0, 0);
      /* force a complete garbage collection in case of errors */
      if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
    }

  m_interp.unlock();
  return status;
}
