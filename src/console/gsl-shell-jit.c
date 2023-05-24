/*
 * GSL shell interactive interface to GSL library
 * Based on the Lua programming language
 *
 * Copyright (C) 2009-2013 Francesco Abbate
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Major portions taken verbatim or adapted from LuaJIT2 frontend.
 * Copyright (C) 2005-2010 Mike Pall. See Copyright Notice in luajit.h
 *
 * Major portions taken verbatim or adapted from the Lua interpreter.
 * Copyright (C) 1994-2008 Lua.org, PUC-Rio. See Copyright Notice in lua.h
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#define luajit_c

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"
#include "lua-filesystem.h"
#include "lua-gsl.h"
#include "gsl-shell.h"
#include "completion.h"
#include "platform.h"
#ifndef NO_GRAPHICS
#include "lua-graph.h"
#include "window_hooks.h"
#include "window.h"
#endif

#if defined(USE_READLINE)
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif

#if defined(LUA_USE_POSIX)
#include <unistd.h>
#define lua_stdin_is_tty()	isatty(0)
#elif defined(LUA_USE_WIN)
#include <io.h>
#ifdef __BORLANDC__
#define lua_stdin_is_tty()	isatty(_fileno(stdin))
#else
#define lua_stdin_is_tty()	_isatty(_fileno(stdin))
#endif
#else
#define lua_stdin_is_tty()	1
#endif

#ifdef NO_GRAPHICS
static inline void
gsl_shell_close_with_graph (struct gsl_shell_state* gs, int send_close_req)
{
    lua_close(gs->L);
}

static inline int register_graph(lua_State *L) { return 0; }
#endif

#if defined(USE_READLINE)
static char *
my_readline (lua_State *L, char *b, const char *p)
{
    return readline (p);
}

static void
my_saveline (lua_State *L, int idx)
{
    if (lua_strlen (L, idx) > 0)  /* non-empty line? */
        add_history(lua_tostring(L, idx));  /* add it to history */
}

static void
my_freeline (lua_State *L, char *b)
{
    free (b);
}

#else

static char *
my_readline (lua_State *L, char *b, const char *p)
{
    fputs(p, stdout);
    fflush(stdout);  /* show prompt */
    return fgets(b, LUA_MAXINPUT, stdin);  /* get line */
}

static void my_saveline (lua_State *L, int idx) { }
static void my_freeline (lua_State *L, char *b) { }

#endif

lua_State *globalL = NULL;
struct gsl_shell_state gsl_shell[1];
static const char *progname = LUA_PROGNAME;

#ifndef NO_GRAPHICS
struct window_hooks app_window_hooks[1] = {{
        window_new, window_show, window_attach,
        window_slot_update, window_slot_refresh,
        window_close_wait, window_wait,
        window_save_slot_image, window_restore_slot_image,
        window_register,
    }
};
#endif

static void gsl_shell_openlibs(lua_State *L)
{
    luaopen_gsl (L);
    if (register_graph (L) != 0) {
        fprintf(stderr, "warning: running without graphics module\n");
    }
    luaopen_filesystem (L);
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

static void laction(int i)
{
    signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
			 terminate process (default action) */
    lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static void print_usage(void)
{
    fprintf(stderr,
            "usage: %s [options]... [script [args]...].\n"
            "Available options are:\n"
            "  -e chunk  Execute string " LUA_QL("chunk") ".\n"
            "  -l name   Require library " LUA_QL("name") ".\n"
            "  -b ...    Save or list bytecode.\n"
            "  -j cmd    Perform LuaJIT control command.\n"
            "  -O[opt]   Control LuaJIT optimizations.\n"
            "  -i        Enter interactive mode after executing " LUA_QL("script") ".\n"
            "  -v        Show version information.\n"
            "  -E        Ignore environment variables.\n"
            "  --        Stop handling options.\n"
            "  -         Execute stdin and stop handling options.\n"
            ,
            progname);
    fflush(stderr);
}

static void l_message(const char *pname, const char *msg)
{
    if (pname) fprintf(stderr, "%s: ", pname);
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
}

static int report(lua_State *L, int status)
{
    if (status && !lua_isnil(L, -1)) {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error object is not a string)";
        l_message(progname, msg);
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
    signal(SIGINT, laction);
    status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
    signal(SIGINT, SIG_DFL);
    lua_remove(L, base);  /* remove traceback function */
    /* force a complete garbage collection in case of errors */
    if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
    return status;
}

static void print_version(void)
{
    l_message(NULL, "GSL Shell " GSL_SHELL_RELEASE " -- " GSL_SHELL_COPYRIGHT);
    l_message(NULL, "GNU Scientific Library -- Copyright (C) The GSL Team");
    l_message(NULL, LUAJIT_VERSION " -- " LUAJIT_COPYRIGHT ". " LUAJIT_URL);
}

static void print_jit_status(lua_State *L)
{
    int n;
    const char *s;
    lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
    lua_getfield(L, -1, "jit");  /* Get jit.* module table. */
    lua_remove(L, -2);
    lua_getfield(L, -1, "status");
    lua_remove(L, -2);
    n = lua_gettop(L);
    lua_call(L, 0, LUA_MULTRET);
    fputs(lua_toboolean(L, n) ? "JIT: ON" : "JIT: OFF", stderr);
    for (n++; (s = lua_tostring(L, n)); n++)
        fprintf(stderr, " %s", s);
    fputs("\n", stderr);
}

static void print_help_message()
{
    l_message(NULL, "");
    l_message(NULL, "Documentation available at https://franko.github.io/gsl-shell");
    l_message(NULL, "To obtain help on a function or object type: help(func).");
    l_message(NULL, "Type demo() to see the lists of demo.");
    l_message(NULL, "");
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
    lua_createtable(L, narg, n + 1);
    for (i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i - n);
    }
    return narg;
}

static int dofile(lua_State *L, const char *name)
{
    int status = luaL_loadfile(L, name) || docall(L, 0, 1);
    return report(L, status);
}

static int dostring(lua_State *L, const char *s, const char *name)
{
    int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
    return report(L, status);
}

static int dolibrary(lua_State *L, const char *name)
{
    lua_getglobal(L, "require");
    lua_pushstring(L, name);
    return report(L, docall(L, 1, 1));
}

static const char *
get_prompt(lua_State *L, int firstline)
{
    const char *p;
    lua_getfield(L, LUA_GLOBALSINDEX, firstline ? "_PROMPT" : "_PROMPT2");
    p = lua_tostring(L, -1);
    if (p == NULL) p = firstline ? LUA_PROMPT : LUA_PROMPT2;
    lua_pop(L, 1);  /* remove global */
    return p;
}

static int incomplete(lua_State *L, int status)
{
    if (status == LUA_ERRSYNTAX) {
        size_t lmsg;
        const char *msg = lua_tolstring(L, -1, &lmsg);
        const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
        if (strstr(msg, LUA_QL("<eof>")) == tp) {
            lua_pop(L, 1);
            return 1;
        }
    }
    return 0;  /* else... */
}

static int pushline (lua_State *L, int firstline) {
    char buffer[LUA_MAXINPUT];
    char *b;
    size_t l;
    const char *prmt = get_prompt(L, firstline);

    pthread_mutex_unlock(&gsl_shell->exec_mutex);
    b = my_readline(L, buffer, prmt);
    pthread_mutex_lock(&gsl_shell->exec_mutex);

    if (b == NULL)
        return 0;  /* no input */

    l = strlen(b);
    if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
        b[l-1] = '\0';  /* remove it */
    lua_pushstring(L, b);
    my_freeline(L, b);
    return 1;
}

/* If the input is an expression we load it preceded by "return" so
   that the value is returned as a result of the evaluation.
   If the value is not an expression leave the stack as before and
   returns a non zero value. */
static int yield_expr(lua_State* L, int index, const char* line, size_t len)
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

    lua_pushfstring(L, "return %s", line);
    status = luaL_loadbuffer(L, lua_tostring(L, -1), lua_strlen(L, -1), "=stdin");
    if (status == 0)
    {
        my_saveline(L, index);
        lua_remove(L, -2); /* remove the modified string */
        lua_remove(L, index); /* remove the original string */
        return 0;
    }
    lua_pop(L, 2); /* we pop both the error msg and the modified string */
    return 1;
}

static int loadline(lua_State *L)
{
    int status;
    size_t len;
    const char *line;

    lua_settop(L, 0);
    if (!pushline(L, 1))
        return -1;  /* no input */

    line = lua_tolstring(L, 1, &len);

    if (strcmp (line, "exit") == 0)
        return -1;

    /* try to load the string as an expression */
    if (yield_expr(L, 1, line, len) == 0)
        return 0;

    /* try to load it as a simple Lua chunk */
    status = luaL_loadbuffer(L, line, len, "=stdin");

    if (incomplete(L, status))
    {
        for (;;) {  /* repeat until gets a complete line */
            if (!pushline(L, 0))  /* no more input? */
                return -1;
            lua_pushliteral(L, "\n");  /* add a new line... */
            lua_insert(L, -2);  /* ...between the two lines */
            lua_concat(L, 3);  /* join them */

            line = lua_tolstring(L, 1, &len);
            status = luaL_loadbuffer(L, line, len, "=stdin");
            if (!incomplete(L, status)) break;  /* cannot try to add lines? */
        }

        /* even if previous "load" was successfull we try to load the string as
        an expression */
        if (yield_expr(L, 1, line, len) == 0)
        {
            /* remove old eval function */
            lua_remove(L, 1);
            return 0;
        }
    }

    my_saveline(L, 1);
    lua_remove(L, 1);  /* remove line */
    return status;
}

static void dotty(lua_State *L)
{
    int status;
    const char *oldprogname = progname;
    progname = NULL;
    while ((status = loadline(L)) != -1) {
        if (status == 0) status = docall(L, 0, 0);
        report(L, status);
        if (status == 0 && lua_gettop(L) > 0) {  /* any result to print? */
            lua_pushvalue(L, -1);
            lua_setfield(L, LUA_GLOBALSINDEX, "_");

            lua_getglobal(L, "print");
            lua_insert(L, 1);
            if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
                l_message(progname,
                          lua_pushfstring(L, "error calling " LUA_QL("print") " (%s)",
                                          lua_tostring(L, -1)));
        }
    }
    lua_settop(L, 0);  /* clear stack */
    fputs("\n", stdout);
    fflush(stdout);
    progname = oldprogname;
}

static int handle_script(lua_State *L, char **argv, int n)
{
    int status;
    const char *fname;
    int narg = getargs(L, argv, n);  /* collect arguments */
    lua_setglobal(L, "arg");
    fname = argv[n];
    if (strcmp(fname, "-") == 0 && strcmp(argv[n-1], "--") != 0)
        fname = NULL;  /* stdin */
    status = luaL_loadfile(L, fname);
    lua_insert(L, -(narg+1));
    if (status == 0)
        status = docall(L, narg, 0);
    else
        lua_pop(L, narg);
    return report(L, status);
}

/* Load add-on module. */
static int loadjitmodule(lua_State *L)
{
    lua_getglobal(L, "require");
    lua_pushliteral(L, "jit.");
    lua_pushvalue(L, -3);
    lua_concat(L, 2);
    if (lua_pcall(L, 1, 1, 0)) {
        const char *msg = lua_tostring(L, -1);
        if (msg && !strncmp(msg, "module ", 7)) {
err:
            l_message(progname,
                      "unknown luaJIT command or jit.* modules not installed");
            return 1;
        } else {
            return report(L, 1);
        }
    }
    lua_getfield(L, -1, "start");
    if (lua_isnil(L, -1)) goto err;
    lua_remove(L, -2);  /* Drop module table. */
    return 0;
}

/* Run command with options. */
static int runcmdopt(lua_State *L, const char *opt)
{
    int narg = 0;
    if (opt && *opt) {
        for (;;) {  /* Split arguments. */
            const char *p = strchr(opt, ',');
            narg++;
            if (!p) break;
            if (p == opt)
                lua_pushnil(L);
            else
                lua_pushlstring(L, opt, (size_t)(p - opt));
            opt = p + 1;
        }
        if (*opt)
            lua_pushstring(L, opt);
        else
            lua_pushnil(L);
    }
    return report(L, lua_pcall(L, narg, 0, 0));
}

/* JIT engine control command: try jit library first or load add-on module. */
static int dojitcmd(lua_State *L, const char *cmd)
{
    const char *opt = strchr(cmd, '=');
    lua_pushlstring(L, cmd, opt ? (size_t)(opt - cmd) : strlen(cmd));
    lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
    lua_getfield(L, -1, "jit");  /* Get jit.* module table. */
    lua_remove(L, -2);
    lua_pushvalue(L, -2);
    lua_gettable(L, -2);  /* Lookup library function. */
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);  /* Drop non-function and jit.* table, keep module name. */
        if (loadjitmodule(L))
            return 1;
    } else {
        lua_remove(L, -2);  /* Drop jit.* table. */
    }
    lua_remove(L, -2);  /* Drop module name. */
    return runcmdopt(L, opt ? opt+1 : opt);
}

/* Optimization flags. */
static int dojitopt(lua_State *L, const char *opt)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
    lua_getfield(L, -1, "jit.opt");  /* Get jit.opt.* module table. */
    lua_remove(L, -2);
    lua_getfield(L, -1, "start");
    lua_remove(L, -2);
    return runcmdopt(L, opt);
}

/* Save or list bytecode. */
static int dobytecode(lua_State *L, char **argv)
{
    int narg = 0;
    lua_pushliteral(L, "bcsave");
    if (loadjitmodule(L))
        return 1;
    if (argv[0][2]) {
        narg++;
        argv[0][1] = '-';
        lua_pushstring(L, argv[0]+1);
    }
    for (argv++; *argv != NULL; narg++, argv++)
        lua_pushstring(L, *argv);
    return report(L, lua_pcall(L, narg, 0, 0));
}

/* check that argument has no extra characters at the end */
#define notail(x)	{if ((x)[2] != '\0') return -1;}

#define FLAGS_INTERACTIVE	1
#define FLAGS_VERSION		2
#define FLAGS_EXEC		4
#define FLAGS_OPTION		8
#define FLAGS_NOENV		16

static int collectargs(char **argv, int *flags)
{
    int i;
    for (i = 1; argv[i] != NULL; i++) {
        if (argv[i][0] != '-')  /* Not an option? */
            return i;
        switch (argv[i][1]) {  /* Check option. */
        case '-':
            notail(argv[i]);
            return (argv[i+1] != NULL ? i+1 : 0);
        case '\0':
            return i;
        case 'i':
            notail(argv[i]);
            *flags |= FLAGS_INTERACTIVE;
            /* fallthrough */
        case 'v':
            notail(argv[i]);
            *flags |= FLAGS_VERSION;
            break;
        case 'e':
            *flags |= FLAGS_EXEC;
        case 'j':  /* LuaJIT extension */
        case 'l':
            *flags |= FLAGS_OPTION;
            if (argv[i][2] == '\0') {
                i++;
                if (argv[i] == NULL) return -1;
            }
            break;
        case 'O':
            break;  /* LuaJIT extension */
        case 'b':  /* LuaJIT extension */
            if (*flags) return -1;
            *flags |= FLAGS_EXEC;
            return 0;
        case 'E':
            *flags |= FLAGS_NOENV;
            break;
        default:
            return -1;  /* invalid option */
        }
    }
    return 0;
}

static int runargs(lua_State *L, char **argv, int n)
{
    int i;
    for (i = 1; i < n; i++) {
        if (argv[i] == NULL) continue;
        lua_assert(argv[i][0] == '-');
        switch (argv[i][1]) {  /* option */
        case 'e': {
            const char *chunk = argv[i] + 2;
            if (*chunk == '\0') chunk = argv[++i];
            lua_assert(chunk != NULL);
            if (dostring(L, chunk, "=(command line)") != 0)
                return 1;
            break;
        }
        case 'l': {
            const char *filename = argv[i] + 2;
            if (*filename == '\0') filename = argv[++i];
            lua_assert(filename != NULL);
            if (dolibrary(L, filename))
                return 1;  /* stop if file fails */
            break;
        }
        case 'j': {  /* LuaJIT extension */
            const char *cmd = argv[i] + 2;
            if (*cmd == '\0') cmd = argv[++i];
            lua_assert(cmd != NULL);
            if (dojitcmd(L, cmd))
                return 1;
            break;
        }
        case 'O':  /* LuaJIT extension */
            if (dojitopt(L, argv[i] + 2))
                return 1;
            break;
        case 'b':  /* LuaJIT extension */
            return dobytecode(L, argv+i);
        default:
            break;
        }
    }
    return 0;
}

static int handle_luainit(lua_State *L)
{
    const char *init = getenv(LUA_INIT);
    if (init == NULL)
        return 0;  /* status OK */
    else if (init[0] == '@')
        return dofile(L, init+1);
    else
        return dostring(L, init, "=" LUA_INIT);
}

static struct Smain {
    char **argv;
    int argc;
    int status;
    int keep_windows;
} smain;

static int pmain(lua_State *L)
{
    struct Smain *s = &smain;
    char **argv = s->argv;
    int script;
    int flags = 0;
    globalL = L;
    if (argv[0] && argv[0][0]) progname = argv[0];
    LUAJIT_VERSION_SYM();  /* linker-enforced version check */
    script = collectargs(argv, &flags);
    if (script < 0) {  /* invalid args? */
        print_usage();
        s->status = 1;
        return 0;
    }
    if ((flags & FLAGS_NOENV)) {
        lua_pushboolean(L, 1);
        lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
    }
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */

    char exename[2048];
    get_exe_filename(exename, sizeof(exename));
    lua_pushstring(L, exename);
    lua_setglobal(L, "EXEFILE");

    gsl_shell_openlibs(L);
    lua_gc(L, LUA_GCRESTART, -1);
    run_start_script(L);

    s->keep_windows = 1;
    if (!(flags & FLAGS_NOENV)) {
        s->status = handle_luainit(L);
        if (s->status != 0) return 0;
    }
    if (s->status != 0) return 0;
    if ((flags & FLAGS_VERSION)) print_version();
    s->status = runargs(L, argv, (script > 0) ? script : s->argc);
    if (s->status != 0) return 0;
    if (script) {
        s->status = handle_script(L, argv, script);
        if (s->status != 0) return 0;
    }
    if ((flags & FLAGS_INTERACTIVE)) {
        print_jit_status(L);
        dotty(L);
        s->keep_windows = 0;
    } else if (script == 0 && !(flags & (FLAGS_EXEC|FLAGS_VERSION))) {
        if (lua_stdin_is_tty()) {
            print_version();
            print_jit_status(L);
            print_help_message();
            dotty(L);
            s->keep_windows = 0;
        } else {
            dofile(L, NULL);  /* executes stdin as a file */
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    int status;
#ifdef _WIN32
  HINSTANCE lib = LoadLibrary("user32.dll");
  int (*SetProcessDPIAware)() = (void*) GetProcAddress(lib, "SetProcessDPIAware");
  SetProcessDPIAware();
#endif

#ifdef USE_READLINE
    initialize_readline();
#endif
    gsl_shell_init(gsl_shell);
    gsl_shell_open(gsl_shell);

    pthread_mutex_lock(&gsl_shell->exec_mutex);

    smain.argc = argc;
    smain.argv = argv;
    status = lua_cpcall(gsl_shell->L, pmain, NULL);
    report(gsl_shell->L, status);

    pthread_mutex_unlock(&gsl_shell->exec_mutex);

    gsl_shell_close_with_graph(gsl_shell, !smain.keep_windows);
    gsl_shell_free(gsl_shell);

    return (status || smain.status) ? EXIT_FAILURE : EXIT_SUCCESS;
}
