#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

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

pthread_mutex_t gsl_shell_mutex[1];

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

void
gsl_shell_close (lua_State *L)
{
  lua_close (L);
  pthread_mutex_destroy (gsl_shell_mutex);
}

lua_State *
gsl_shell_init ()
{
  lua_State *L;
  int status;

  pthread_mutex_init (gsl_shell_mutex, NULL);

  L = lua_open();  /* create state */

  if (L == NULL)
    {
      fprintf(stderr, "cannot create state: not enough memory");
      return NULL;
    }

  status = lua_cpcall(L, pinit, NULL);

  if (report(L, status))
    {
      gsl_shell_close (L);
      return NULL;
    }

  return L;
}

int
gsl_shell_exec (lua_State *L, const char *line)
{
  int status;
  pthread_mutex_lock (gsl_shell_mutex);
  status = luaL_loadbuffer(L, line, strlen(line), "=<user input>");
  if (status == 0)
    {
      status = lua_pcall(L, 0, 0, 0);
      /* force a complete garbage collection in case of errors */
      if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
    }
  pthread_mutex_unlock (gsl_shell_mutex);
  return status;
}
