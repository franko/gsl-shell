#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define luajit_c

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"
}

#include "fatal.h"
#include "gsl_shell_interp.h"
#include "lua-filesystem.h"
#include "lua-graph.h"
#include "lua-gsl.h"
#include "platform.h"

static void stderr_message(const char *pname, const char *msg)
{
    if (pname) fprintf(stderr, "%s: ", pname);
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
}

static int stderr_report(lua_State *L, int status)
{
    if (status && !lua_isnil(L, -1))
    {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error object is not a string)";
        stderr_message("gsl-shell", msg);
        lua_pop(L, 1);
    }
    return status;
}

static int traceback(lua_State *L)
{
    if (!lua_isstring(L, 1))  /* 'message' not a string? */
        return 1;  /* keep it intact */
    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return 1;
    }
    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 2);
        return 1;
    }
    lua_pushvalue(L, 1);  /* pass error message */
    lua_pushinteger(L, 2);  /* skip this function and traceback */
    lua_call(L, 2, 1);  /* call debug.traceback */
    return 1;
}

static int docall(lua_State *L, int narg, int clear)
{
    int status;
    int base = lua_gettop(L) - narg;  /* function index */
    lua_pushcfunction(L, traceback);  /* push traceback function */
    lua_insert(L, base);  /* put it under chunk and args */
    status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
    lua_remove(L, base);  /* remove traceback function */
    /* force a complete garbage collection in case of errors */
    if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
    return status;
}

static int pinit(lua_State *L)
{
    LUAJIT_VERSION_SYM();  /* linker-enforced version check */
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */

    char exename[2048];
    get_exe_filename(exename, sizeof(exename));
    lua_pushstring(L, exename);
    lua_setglobal(L, "EXEFILE");

    luaopen_gsl (L);
    register_graph (L);
    luaopen_filesystem (L);
    lua_gc(L, LUA_GCRESTART, -1);
    run_start_script(L);
    return 0;
}

/* If the input is an expression we load it preceded by "return" so
   that the value is returned as a result of the evaluation.
   If the value is not an expression leave the stack as before and
   returns a non zero value. */
static int yield_expr(lua_State* L, const char* line, size_t len)
{
    const char *p;
    int status;

    for (p = line + len - 1; p >= line; p--)
    {
        const char c = *p;
        if (c == ';')
            return 1;
        if (c != ' ')
            break;
    }

    str mline = str::print("return %s", line);
    status = luaL_loadbuffer(L, mline.cstr(), len+7, "=stdin");
    if (status != 0) lua_pop(L, 1); // remove the error message
    return status;
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

void gsl_shell::init()
{
    gsl_shell_open(this);

    int status = lua_cpcall(this->L, pinit, NULL);

    if (unlikely(stderr_report(this->L, status)))
    {
        lua_close(this->L);
        fatal_exception("cannot initialize Lua state");
    }
}

void gsl_shell::close()
{
    lua_close(this->L);
    this->L = NULL;
}

int gsl_shell::error_report(int status)
{
    lua_State* L = this->L;
    if (status && !lua_isnil(L, -1))
    {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error object is not a string)";
        m_error_msg = msg;
        lua_pop(L, 1);
    }
    return status;
}

int gsl_shell::exec(const char *line)
{
    lua_State* L = this->L;
    size_t len = strlen(line);

    /* try to load the string as an expression */
    int status = yield_expr(L, line, len);

    if (status != 0)
    {
        status = luaL_loadbuffer(L, line, len, "=<user input>");

        if (incomplete(L, status))
            return incomplete_input;
    }

    if (status == 0)
    {
        status = docall(L, 0, 0);
        error_report(status);
        if (status == 0 && lua_gettop(L) > 0)   /* any result to print? */
        {
            lua_pushvalue(L, -1);
            lua_setfield(L, LUA_GLOBALSINDEX, "_");

            lua_getglobal(L, "print");
            lua_insert(L, 1);
            if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
                fprintf(stderr, "error calling print function");
        }
    }

    error_report(status);

    return (status == 0 ? eval_success : eval_error);
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

void
gsl_shell::interrupt()
{
    lua_sethook(this->L, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}
