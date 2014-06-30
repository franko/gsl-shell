#include <string.h>

#include "gsl_shell_interp.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lua-gsl.h"
#include "lua-utils.h"
#include "lang/language.h"
#include "lang/lua-language-gs.h"

gsl_shell_interp *global_gs;

static int report_error_msg(gsl_shell_interp *gs, int status)
{
    lua_State* L = gs->L;
    if (status && !lua_isnil(L, -1))
    {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error object is not a string)";
        if (status == LUA_LANGERR) {
            str_copy_c(gs->m_error_msg, "gsl-shell internal error:");
            str_append_c(gs->m_error_msg, msg, ' ');
        } else {
            str_copy_c(gs->m_error_msg, msg);
        }
        lua_pop(L, 1);
    }
    return status;
}

static int traceback(lua_State *L)
{
    if (!lua_isstring(L, 1)) { /* Non-string error object? Try metamethod. */
        if (lua_isnoneornil(L, 1) ||
                !luaL_callmeta(L, 1, "__tostring") ||
                !lua_isstring(L, -1))
            return 1;  /* Return non-string error object. */
        lua_remove(L, 1);  /* Replace object by result of __tostring metamethod. */
    }
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
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

static int dolibrary(lua_State *L, const char *name)
{
    lua_getglobal(L, "require");
    lua_pushstring(L, name);
    return docall(L, 1, 1);
}

/* Lua C function that perform initialization. */
static int pinit(lua_State *L)
{
    int status;
    LUAJIT_VERSION_SYM();  /* linker-enforced version check */
    gsl_shell_interp *gs = lua_touserdata(L, 1);

    status = language_init(L);
    if (status != 0) {
        report_error_msg(gs, status);
        return status;
    }

    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */
    /* TODO: perform other library using a use defined callback. */
    luaopen_gsl(L);
    if (gs->graphics) {
        gs->graphics->init(L);
    }
    luaopen_language(L);
    lua_gc(L, LUA_GCRESTART, -1);
    status = dolibrary(L, "gslext");
    return status;
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

    char *mline = malloc(len + 7 + 1);
    if (mline == NULL) {
        return 1;
    }
    memcpy(mline, "return ", 7);
    memcpy(mline + 7, line, len + 1);
    status = language_loadbuffer(L, mline, len + 7, "=stdin");
    free(mline);
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

void
gsl_shell_interp_init(gsl_shell_interp *gs)
{
    gs->L = NULL;
    gs->graphics = NULL;
    pthread_mutex_init(&gs->exec_mutex, NULL);
    pthread_mutex_init(&gs->shutdown_mutex, NULL);
    str_init(gs->m_error_msg, 64);
}

void
gsl_shell_interp_free(gsl_shell_interp *gs)
{
    pthread_mutex_destroy (&gs->exec_mutex);
    pthread_mutex_destroy (&gs->shutdown_mutex);
    str_free(gs->m_error_msg);
}

int
gsl_shell_interp_open(gsl_shell_interp *gs, const graphics_lib *graphics)
{
    gs->L = lua_open();  /* create state */

    if (unlikely(gs->L == NULL)) {
        str_copy_c(gs->m_error_msg, "cannot create state: not enough memory");
        return LUA_ERRRUN;
    }

    gs->graphics = graphics;

    int status = lua_cpcall(gs->L, pinit, gs);

    if (status != 0)
    {
        lua_close(gs->L);
        gs->L = NULL;
        return LUA_ERRRUN;
    }

    gs->is_shutting_down = 0;
    global_gs = gs;
    return 0;
}

void
gsl_shell_interp_lock(gsl_shell_interp *gs)
{
    pthread_mutex_lock(&gs->exec_mutex);
}

void
gsl_shell_interp_unlock(gsl_shell_interp *gs)
{
    pthread_mutex_unlock(&gs->exec_mutex);
}

int
gsl_shell_interp_exec(gsl_shell_interp *gs, const char *line)
{
    lua_State* L = gs->L;
    size_t len = strlen(line);

    /* try to load the string as an expression */
    int status = yield_expr(L, line, len);

    if (status == LUA_LANGERR) { /* Internal language error */
        /* We fail early because it is an internal error of the
         * language implementation. */
        report_error_msg(gs, status);
        return status;
    } else if (status != 0) {
        lua_pop(L, 1); /* Pop error message. */
    }

    if (status != 0)
    {
        status = language_loadbuffer(L, line, len, "=<user input>");

        if (incomplete(L, status))
            return status + (incomplete_input << 8);
    }

    if (status == 0)
    {
        status = docall(L, 0, 0);
        report_error_msg(gs, status);
        if (status == 0 && lua_gettop(L) > 0)   /* any result to print? */
        {
            lua_pushvalue(L, -1);
            lua_setfield(L, LUA_GLOBALSINDEX, "_");

            lua_getglobal(L, "print");
            lua_insert(L, 1);
            if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
                fprintf(stderr, "error calling print function");
        }
    } else {
        report_error_msg(gs, status);
    }

    return status;
}

int
gsl_shell_interp_dostring(gsl_shell_interp *gs, const char *s, const char *name)
{
    int status = language_loadbuffer(gs->L, s, strlen(s), name);
    if (status == 0) {
        status = docall(gs->L, 0, 1);
    };
    report_error_msg(gs, status);
    return status;
}

int
gsl_shell_interp_dofile(gsl_shell_interp *gs, const char *name)
{
    int status = language_loadfile(gs->L, name);
    if (status == 0) {
        status = docall(gs->L, 0, 1);
    };
    report_error_msg(gs, status);
    return status;
}

int
gsl_shell_interp_dolibrary(gsl_shell_interp *gs, const char *name)
{
    lua_State *L = gs->L;
    lua_getglobal(L, "require");
    lua_pushstring(L, name);
    int status = docall(L, 1, 1);
    report_error_msg(gs, status);
    return status;
}

static int getargs(lua_State *L, char **argv, int n)
{
    int narg;
    int i;
    int argc = 0;
    while (argv[argc]) argc++;  /* count total number of arguments */
    narg = argc - (n + 1);  /* number of arguments to the script */
    luaL_checkstack(L, narg + 3, "too many arguments to script");
    for (i = n+1; i < argc; i++)
        lua_pushstring(L, argv[i]);
    return narg;
}

int
gsl_shell_interp_doscript(gsl_shell_interp *gs, char **argv, int n)
{
    lua_State *L = gs->L;
    int status;
    const char *fname;
    int narg = getargs(L, argv, n);  /* collect arguments */
    fname = argv[n];
    if (strcmp(fname, "-") == 0 && strcmp(argv[n-1], "--") != 0)
        fname = NULL;  /* stdin */
    status = language_loadfile(L, fname);
    lua_insert(L, -(narg+1));
    if (status == 0) {
        status = docall(L, narg, 0);
    } else {
        lua_pop(L, narg);
    }
    return report_error_msg(gs, status);
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
gsl_shell_interp_interrupt(gsl_shell_interp *gs)
{
    lua_sethook(gs->L, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

const char *
gsl_shell_interp_error(const gsl_shell_interp *gs)
{
    return CSTR(gs->m_error_msg);
}

void
gsl_shell_interp_close(gsl_shell_interp *gs)
{
    lua_close(gs->L);
    gs->L = NULL;
}

void
gsl_shell_interp_close_with_graph (gsl_shell_interp* gs, int send_close_req)
{
    pthread_mutex_lock (&gs->shutdown_mutex);
    gs->is_shutting_down = 1;
    pthread_mutex_lock(&gs->exec_mutex);
    if (send_close_req) {
        gs->graphics->close_windows(gs->L);
    } else {
        gs->graphics->wait_windows(gs->L);
    }
    lua_close(gs->L);
    gs->L = NULL;
    pthread_mutex_unlock(&gs->shutdown_mutex);
    pthread_mutex_unlock(&gs->exec_mutex);
}
