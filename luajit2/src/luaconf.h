/*
** Configuration header.
** Copyright (C) 2005-2011 Mike Pall. See Copyright Notice in luajit.h
*/

#ifndef luaconf_h
#define luaconf_h

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)

#include <limits.h>
#include <stddef.h>

/* Try to determine supported features for a couple of standard platforms. */
#if defined(_WIN32)
#define LUA_USE_WIN
#define LUA_DL_DLL
#elif defined(__linux__) || defined(__solaris__) || defined(__CYGWIN__) || \
      defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
      defined(__FreeBSD_kernel__) || (defined(__MACH__) && defined(__APPLE__))
#define LUA_USE_POSIX
#define LUA_DL_DLOPEN
#define LUA_USE_READLINE
#endif

/* Default path for loading Lua and C modules with require(). */
#ifndef LUA_STRICT 
  #define STDLIB_NAME "gsl-shell"
#else
  #define STDLIB_NAME "lua"
#endif
#ifdef LUA_USE_WIN
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#define LUA_LDIR	"!\\" STDLIB_NAME "\\"
#define LUA_CDIR	"!\\"
#define LUA_PATH_DEFAULT \
  ".\\?.lua;" ".\\templates\\?.lua.in;" LUA_LDIR"?.lua;" LUA_LDIR"?\\init.lua;" LUA_CDIR"templates\\?.lua.in"
#define LUA_CPATH_DEFAULT \
  ".\\?.dll;" LUA_CDIR"?.dll;" LUA_CDIR"loadall.dll"
#else
#define LUA_ROOT_Q      QUOTEME(LUA_ROOT) "/"
#define LUA_LDIR	LUA_ROOT_Q "share/" STDLIB_NAME "/"
#define LUA_CDIR	LUA_ROOT_Q "lib/" STDLIB_NAME  "/"
#define LUA_PATH_DEFAULT "./?.lua;" "./templates/?.lua.in;" \
                         LUA_LDIR"?.lua;" LUA_LDIR"?/init.lua;" \
                         LUA_CDIR"?.lua;" LUA_CDIR"?/init.lua;" \
                         LUA_CDIR"templates/?.lua.in"
#define LUA_CPATH_DEFAULT "./?.so;" LUA_CDIR"?.so;" LUA_CDIR"loadall.so"
#endif

/* Environment variable names for path overrides and initialization code. */
#define LUA_PATH	"LUA_PATH"
#define LUA_CPATH	"LUA_CPATH"
#define LUA_INIT	"LUA_INIT"

/* Special file system characters. */
#if defined(_WIN32)
#define LUA_DIRSEP	"\\"
#else
#define LUA_DIRSEP	"/"
#endif
#define LUA_PATHSEP	";"
#define LUA_PATH_MARK	"?"
#define LUA_EXECDIR	"!"
#define LUA_IGMARK	"-"
#define LUA_PATH_CONFIG \
  LUA_DIRSEP "\n" LUA_PATHSEP "\n" LUA_PATH_MARK "\n" \
  LUA_EXECDIR "\n" LUA_IGMARK

/* Quoting in error messages. */
#define LUA_QL(x)	"'" x "'"
#define LUA_QS		LUA_QL("%s")

/* Various tunables. */
#define LUAI_MAXSTACK	65500	/* Max. # of stack slots for a thread (<64K). */
#define LUAI_MAXCSTACK	8000	/* Max. # of stack slots for a C func (<10K). */
#define LUAI_GCPAUSE	200	/* Pause GC until memory is at 200%. */
#define LUAI_GCMUL	200	/* Run GC at 200% of allocation speed. */
#define LUA_MAXCAPTURES	32	/* Max. pattern captures. */

/* Compatibility with older library function names. */
#define LUA_COMPAT_MOD		/* OLD: math.mod, NEW: math.fmod */
#define LUA_COMPAT_GFIND	/* OLD: string.gfind, NEW: string.gmatch */

/* Configuration for the frontend (the luajit executable). */
#if defined(luajit_c)
#define LUA_PROGNAME	"luajit"  /* Fallback frontend name. */
#define LUA_PROMPT	"> "	/* Interactive prompt. */
#define LUA_PROMPT2	">> "	/* Continuation prompt. */
#define LUA_MAXINPUT	512	/* Max. input line length. */
#endif

/*
@@ lua_readline defines how to show a prompt and then read a line from
@* the standard input.
@@ lua_saveline defines how to "save" a read line in a "history".
@@ lua_freeline defines how to free a line read by lua_readline.
** CHANGE them if you want to improve this functionality (e.g., by using
** GNU readline and history facilities).
*/
#if defined(LUA_USE_READLINE)
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#define lua_readline(L,b,p)	((void)L, ((b)=readline(p)) != NULL)
#define lua_saveline(L,idx) \
	if (lua_strlen(L,idx) > 0)  /* non-empty line? */ \
	  add_history(lua_tostring(L, idx));  /* add it to history */
#define lua_freeline(L,b)	((void)L, free(b))
#else
#define lua_readline(L,b,p)	\
	((void)L, fputs(p, stdout), fflush(stdout),  /* show prompt */ \
	fgets(b, LUA_MAXINPUT, stdin) != NULL)  /* get line */
#define lua_saveline(L,idx)	{ (void)L; (void)idx; }
#define lua_freeline(L,b)	{ (void)L; (void)b; }
#endif

/* Note: changing the following defines breaks the Lua 5.1 ABI. */
#define LUA_INTEGER	ptrdiff_t
#define LUA_IDSIZE	60	/* Size of lua_Debug.short_src. */
#define LUAL_BUFFERSIZE	BUFSIZ	/* Size of lauxlib and io.* buffers. */

/* The following defines are here only for compatibility with luaconf.h
** from the standard Lua distribution. They must not be changed for LuaJIT.
*/
#define LUA_NUMBER_DOUBLE
#define LUA_NUMBER		double
#define LUAI_UACNUMBER		double
#define LUA_NUMBER_SCAN		"%lf"
#define LUA_NUMBER_FMT		"%.14g"
#define lua_number2str(s, n)	sprintf((s), LUA_NUMBER_FMT, (n))
#define LUAI_MAXNUMBER2STR	32
#define lua_str2number(s, p)	strtod((s), (p))
#define LUA_INTFRMLEN		"l"
#define LUA_INTFRM_T		long

/* Linkage of public API functions. */
#if defined(LUA_BUILD_AS_DLL)
#if defined(LUA_CORE) || defined(LUA_LIB)
#define LUA_API		__declspec(dllexport)
#else
#define LUA_API		__declspec(dllimport)
#endif
#else
#define LUA_API		extern
#endif

#define LUALIB_API	LUA_API

/* Support for internal assertions. */
#if defined(LUA_USE_ASSERT) || defined(LUA_USE_APICHECK)
#include <assert.h>
#endif
#ifdef LUA_USE_ASSERT
#define lua_assert(x)		assert(x)
#endif
#ifdef LUA_USE_APICHECK
#define luai_apicheck(L, o)	{ (void)L; assert(o); }
#else
#define luai_apicheck(L, o)	{ (void)L; }
#endif

#endif
