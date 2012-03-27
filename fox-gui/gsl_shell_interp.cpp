#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define luajit_c

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"
}

#include "lua-gsl.h"
#include "lua-graph.h"

#include  "gsl_shell_interp.h"
#include "fatal.h"

lua_State *globalL = NULL;

static void stderr_message(const char *pname, const char *msg)
{
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}

static int stderr_report(lua_State *L, int status)
{
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    stderr_message("gsl-shell", msg);
    lua_pop(L, 1);
  }
  return status;
}

static void lstop(lua_State *L, lua_Debug *ar)
{
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  /* Avoid luaL_error -- a C hook doesn't add an extra frame. */
  luaL_where(L, 0);
  lua_pushfstring(L, "%sinterrupted!", lua_tostring(L, -1));
  lua_error(L);
}

static int traceback(lua_State *L)
{
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}

static void laction(int i)
{
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
			 terminate process (default action) */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static int docall(lua_State *L, int narg, int clear)
{
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}

static int dolibrary(lua_State *L, const char *name)
{
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  return stderr_report(L, docall(L, 1, 1));
}

static int pinit(lua_State *L)
{
  LUAJIT_VERSION_SYM();  /* linker-enforced version check */
  lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
  luaL_openlibs(L);  /* open libraries */
  luaopen_gsl (L);
  register_graph (L);
  lua_gc(L, LUA_GCRESTART, -1);
  dolibrary (L, "gslext");
  return 0;
}

static int incomplete(lua_State *L, int status)
{
  if (status == LUA_ERRSYNTAX)
    {
      size_t lmsg;
      const char *msg = lua_tolstring(L, -1, &lmsg);
      const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
      if (strstr(msg, LUA_QL("<eof>")) == tp)
	{
	  lua_pop(L, 1);
	  return 1;
	}
    }
  return 0;  /* else... */
}

gsl_shell::~gsl_shell()
{
  if (m_lua_state)
    {
      lua_close_with_graph(m_lua_state);
      gsl_shell_close();
    }
}

void gsl_shell::init()
{
  lua_State *L;
  int status;

  GSL_SHELL_LOCK();
  gsl_shell_init();
  L = lua_open();  /* create state */
  globalL = L;

  if (unlikely(L == NULL))
    fatal_exception("cannot create state: not enough memory");

  status = lua_cpcall(L, pinit, NULL);

  if (unlikely(report(L, status)))
    {
      lua_close(L);
      fatal_exception("cannot initialize Lua state");
    }

  m_lua_state = L;
}

int gsl_shell::report(lua_State* L, int status)
{
  if (status && !lua_isnil(L, -1))
    {
      const char *msg = lua_tostring(L, -1);
      if (msg == NULL) msg = "(error object is not a string)";

      int np = snprintf(m_error_msg, ERROR_MSG_MAX_LENGTH, "%s", msg);
      if (np >= ERROR_MSG_MAX_LENGTH)
	m_error_msg[ERROR_MSG_MAX_LENGTH - 1] = 0;

      lua_pop(L, 1);
    }
  return status;
}

int gsl_shell::exec(const char *line)
{
  pthread::auto_lock lock(m_interp);
  lua_State* L = m_lua_state;

  int status = luaL_loadbuffer(L, line, strlen(line), "=<user input>");

  if (incomplete(L, status))
    return incomplete_input;

  if (status == 0)
    {
      status = lua_pcall(L, 0, 0, 0);
      /* force a complete garbage collection in case of errors */
      if (status != 0)
	lua_gc(L, LUA_GCCOLLECT, 0);
    }

  report(L, status);
  GSL_SHELL_UNLOCK();

  return (status == 0 ? eval_success : eval_error);
}

void gsl_shell::lock()
{
  GSL_SHELL_LOCK();
}

void gsl_shell::unlock()
{
  GSL_SHELL_UNLOCK();
}
