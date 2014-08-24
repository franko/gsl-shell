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

#define luajit_c

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"
#include "completion.h"
#include "lua-gsl.h"
#include "lua-graph.h"
#include "window_hooks.h"
#include "window.h"
#include "gsl-shell.h"
#include "gsl_shell_interp.h"
#include "lang/language.h"

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

lua_State *globalL = NULL;
static gsl_shell_interp *repl_gs = NULL;
static const char *progname = LUA_PROGNAME;

static void l_message(const char *pname, const char *msg)
{
    if (pname) fprintf(stderr, "%s: ", pname);
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
}

#if defined(USE_READLINE)
static char *
my_readline (const char *p)
{
    return readline(p);
}

static void
my_saveline (const char *line)
{
    if (strlen(line) > 0)  /* non-empty line? */
        add_history(line);  /* add it to history */
}

#else

static char *
my_readline (const char *p)
{
    char *str = malloc(LUA_MAXINPUT);
    if (unlikely(str == NULL)) {
        l_message(progname, "memory allocation failed");
        return NULL;
    }
    fputs(p, stdout);
    fflush(stdout);  /* show prompt */
    return fgets(str, LUA_MAXINPUT, stdin);  /* get line */
}

static void my_saveline(const char *line) { }

#endif

struct window_hooks app_window_hooks[1] = {{
        window_new, window_show, window_attach,
        window_slot_update, window_slot_refresh,
        window_close_wait, window_wait,
        window_save_slot_image, window_restore_slot_image,
        window_register,
    }
};

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

static int report(gsl_shell_interp *gs, int status)
{
    if (status) {
        const char *msg = gsl_shell_interp_error(gs);
        l_message(progname, msg);
    }
    return status;
}

static int report_lua(lua_State *L, int status)
{
    if (status && !lua_isnil(L, -1)) {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error object is not a string)";
        l_message(progname, msg);
        lua_pop(L, 1);
    }
    return status;
}

static void print_version(void)
{
    l_message(NULL, "GSL Shell " GSL_SHELL_RELEASE ", Copyright (C) 2009-2013 Francesco Abbate");
    l_message(NULL, "GNU Scientific Library, Copyright (C) The GSL Team");
    l_message(NULL, LUAJIT_VERSION " -- " LUAJIT_COPYRIGHT ". " LUAJIT_URL);
}

static void print_jit_status(lua_State *L)
{
    int n, j;
    const char *s;
    lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
    lua_getfield(L, -1, "jit");  /* Get jit.* module table. */
    lua_remove(L, -2);
    lua_getfield(L, -1, "status");
    lua_remove(L, -2);
    n = lua_gettop(L);
    lua_call(L, 0, LUA_MULTRET);
    fputs(lua_toboolean(L, n) ? "JIT: ON" : "JIT: OFF", stderr);
    for (j = n+1; (s = lua_tostring(L, j)); j++)
        fprintf(stderr, " %s", s);
    fputs("\n", stderr);
    lua_settop(L, n-1);
}

static void print_help_message()
{
    l_message(NULL, "");
    l_message(NULL, "Documentation available at http://www.nongnu.org/gsl-shell/doc/.");
    l_message(NULL, "To obtain help on a function or object type: help(func).");
    l_message(NULL, "Type demo() to see the lists of demo.");
    l_message(NULL, "");
}

static int dofile(gsl_shell_interp *gs, const char *name)
{
    int status = gsl_shell_interp_dofile(gs, name);
    return report(gs, status);
}

static int dostring(gsl_shell_interp *gs, const char *s, const char *name)
{
    int status = gsl_shell_interp_dostring(gs, s, name);
    return report(gs, status);
}

static int dolibrary(gsl_shell_interp *gs, const char *name)
{
    int status = gsl_shell_interp_dolibrary(gs, name);
    return report(gs, status);
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

static char *my_getline(gsl_shell_interp *gs, int firstline) {
    char *b;
    size_t l;
    const char *prmt = get_prompt(gs->L, firstline);

    gsl_shell_interp_unlock(gs);
    b = my_readline(prmt);
    gsl_shell_interp_lock(gs);

    if (b == NULL)
        return 0;  /* no input */

    l = strlen(b);
    if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
        b[l-1] = '\0';  /* remove it */
    return b;
}

/* Create a new strings by concatenating "a" and "b" and
   free the input strings. */
static char *concat_string(char *a, char *b, int sep)
{
    size_t len = strlen(a);
    char *accu = malloc(len + strlen(b) + 2);
    if (accu == NULL) {
        goto cat_return;
    }
    memcpy(accu, a, len);
    accu[len] = sep;
    memcpy(accu + len + 1, b, strlen(b) + 1);
cat_return:
    free(a);
    free(b);
    return accu;
}

static void laction(int i)
{
    signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                         terminate process (default action) */
    gsl_shell_interp_interrupt(repl_gs);
}

static void dotty(gsl_shell_interp *gs)
{
    const char *oldprogname = progname;
    progname = NULL;
    int firstline = 1;
    char *line = NULL;
    while (1) {
        char *new_line = my_getline(gs, firstline);
        if (new_line == NULL) {
            free(line); /* Can be NULL. */
            break;
        }
        if (line) {
            line = concat_string(line, new_line, '\n');
        } else {
            line = new_line;
        }
        signal(SIGINT, laction);
        int exec_status = gsl_shell_interp_exec(gs, line);
        signal(SIGINT, SIG_DFL);
        if (exec_status >> 8 == incomplete_input) {
            firstline = 0;
            continue;
        } else if (exec_status >> 8 == exit_command) {
            free(line);
            break;
        } else if (exec_status != 0) {
            const char *msg = gsl_shell_interp_error(gs);
            l_message(progname, msg);
        } else {
            my_saveline(line);
        }
        firstline = 1;
        free(line);
        line = NULL;
    }
    fputs("\n", stdout);
    fflush(stdout);
    progname = oldprogname;
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
            return report_lua(L, 1);
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
    return report_lua(L, lua_pcall(L, narg, 0, 0));
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
    return report_lua(L, lua_pcall(L, narg, 0, 0));
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

static int runargs(gsl_shell_interp *gs, char **argv, int n)
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
            if (dostring(gs, chunk, "=(command line)") != 0)
                return 1;
            break;
        }
        case 'l': {
            const char *filename = argv[i] + 2;
            if (*filename == '\0') filename = argv[++i];
            lua_assert(filename != NULL);
            if (dolibrary(gs, filename))
                return 1;  /* stop if file fails */
            break;
        }
        case 'j': {  /* LuaJIT extension */
            const char *cmd = argv[i] + 2;
            if (*cmd == '\0') cmd = argv[++i];
            lua_assert(cmd != NULL);
            if (dojitcmd(gs->L, cmd))
                return 1;
            break;
        }
        case 'O':  /* LuaJIT extension */
            if (dojitopt(gs->L, argv[i] + 2))
                return 1;
            break;
        case 'b':  /* LuaJIT extension */
            return dobytecode(gs->L, argv+i);
        default:
            break;
        }
    }
    return 0;
}

static int handle_luainit(gsl_shell_interp *gs)
{
    const char *init = getenv(LUA_INIT);
    if (init == NULL)
        return 0;  /* status OK */
    else if (init[0] == '@')
        return dofile(gs, init+1);
    else
        return dostring(gs, init, "=" LUA_INIT);
}

struct Smain {
    char **argv;
    int argc;
    int status;
    gsl_shell_interp *gs;
    int keep_windows;
};

static int pmain(lua_State *L)
{
    struct Smain *s = lua_touserdata(L, 1);
    gsl_shell_interp *gs = s->gs;
    char **argv = s->argv;
    int script;
    int flags = 0;
    globalL = L;
    lua_pop(L, 1); /* Remove from stack Smain pointer. */
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

    s->keep_windows = 1;
    if (!(flags & FLAGS_NOENV)) {
        s->status = handle_luainit(gs);
        if (s->status != 0) return 0;
    }
    if (s->status != 0) return 0;
    if ((flags & FLAGS_VERSION)) print_version();
    s->status = runargs(gs, argv, (script > 0) ? script : s->argc);
    if (s->status != 0) return 0;
    if (script) {
        s->status = gsl_shell_interp_doscript(gs, argv, script);
        report(gs, s->status);
        if (s->status != 0) return 0;
    }
    if ((flags & FLAGS_INTERACTIVE)) {
        print_jit_status(L);
        dotty(gs);
        s->keep_windows = 0;
    } else if (script == 0 && !(flags & (FLAGS_EXEC|FLAGS_VERSION))) {
        if (lua_stdin_is_tty()) {
            print_version();
            print_jit_status(L);
            print_help_message();
            dotty(gs);
            s->keep_windows = 0;
        } else {
            dofile(gs, NULL);  /* executes stdin as a file */
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    int status;
    struct Smain smain[1];
    gsl_shell_interp gs[1];
#ifdef USE_READLINE
    initialize_readline();
#endif
    gsl_shell_interp_init(gs);
    repl_gs = gs;
    status = gsl_shell_interp_open(gs);
    if (status != 0) {
        l_message(progname, "failed to initialize gsl shell");
        return EXIT_FAILURE;
    }

    graphics->init(gs->L); /* Initialize graphics module. */
    luaopen_gsl(gs->L); /* Perform some GSL Shell's specific initializations. */
    status = gsl_shell_interp_dolibrary(gs, "gslext");
    if (status != 0) {
        l_message(progname, "failed to initialize gsl shell extensions");
        return EXIT_FAILURE;
    }

    gsl_shell_interp_lock(gs);

    smain->argc = argc;
    smain->argv = argv;
    smain->gs = gs;
    status = lua_cpcall(gs->L, pmain, smain);
    report(gs, status);

    gsl_shell_interp_unlock(gs);
    if (!smain->keep_windows) {
        graphics->close_windows(gs->L);
    } else {
        graphics->wait_windows(gs->L);
    }
    gsl_shell_interp_close(gs);
    gsl_shell_interp_free(gs);

    return (status || smain->status) ? EXIT_FAILURE : EXIT_SUCCESS;
}
