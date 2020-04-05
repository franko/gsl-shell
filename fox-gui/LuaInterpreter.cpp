#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define luajit_c

#include "LuaInterpreter.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"
#include "language.h"
#include "language_loaders.h"
}

#include "elem/elem_lua.h"

#include "fatal.h"

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

// Function disabled because not currently needed.
// It is otherwise useful and appear in the reference
// Lua frontend.
#if 0
static int dolibrary(lua_State *L, const char *name)
{
    lua_getglobal(L, "require");
    lua_pushstring(L, name);
    return stderr_report(L, docall(L, 1, 1));
}
#endif
static void override_loaders(lua_State *L)
{
  lua_getfield(L, LUA_GLOBALSINDEX, "package");
  lua_getfield(L, -1, "loaders");
  lua_remove(L, -2);

  luaopen_langloaders(L);
  lua_getfield(L, -1, "loadstring");
  lua_setfield(L, LUA_GLOBALSINDEX, "loadstring");

  lua_getfield(L, -1, "loadfile");
  lua_setfield(L, LUA_GLOBALSINDEX, "loadfile");

  lua_getfield(L, -1, "dofile");
  lua_setfield(L, LUA_GLOBALSINDEX, "dofile");

  lua_getfield(L, -1, "loader");
  lua_rawseti(L, -3, 2);
  lua_pop(L, 2);
}

static int loadbuffer_by_language(lua_State *L, LuaLanguage language, const char* line, size_t len, const char *source) {
    int status;
    if (language == LuaLanguage::kStandard) {
        status = luaL_loadbuffer(L, line, len, source);
    } else {
        status = language_loadbuffer(L, line, len, source);
    }
    return status;
}

/* If the input is an expression we load it preceded by "return" so
   that the value is returned as a result of the evaluation.
   If the value is not an expression leave the stack as before and
   returns a non zero value. */
static int yield_expr(lua_State* L, LuaLanguage language, const char* line, size_t len)
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

    std::string yield_string = "return " + std::string{line};
    status = loadbuffer_by_language(L, language, yield_string.c_str(), len + 7, "=<user input>");
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

static int pinit_lang_extension(lua_State *L)
{
    LUAJIT_VERSION_SYM();  /* linker-enforced version check */
    int status = language_init(L);
    if (status != 0) {
        return lua_error(L);
    }
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */
    override_loaders(L);
    lua_gc(L, LUA_GCRESTART, -1);
    return 0;
}

static int pinit_plain_lua(lua_State *L)
{
    LUAJIT_VERSION_SYM();  /* linker-enforced version check */
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */
    lua_gc(L, LUA_GCRESTART, -1);
    return 0;
}

void LuaInterpreter::Initialize() {
    m_lua_state = lua_open();
    int status;
    if (m_lua_language == LuaLanguage::kLanguageExtension) {
        status = lua_cpcall(m_lua_state, pinit_lang_extension, NULL);
    } else {
        status = lua_cpcall(m_lua_state, pinit_plain_lua, NULL);
    }
    if (unlikely(stderr_report(m_lua_state, status))) {
        lua_close(m_lua_state);
        fatal_exception("cannot initialize Lua state");
    }
    // TODO: this bit initializing the Elementary Plot
    // library should be moved elsewhere.
    elem::LuaOpenLibrary(m_lua_state);
}

void LuaInterpreter::Close() {
    lua_close(m_lua_state);
    m_lua_state = nullptr;
}

void LuaInterpreter::LuaErrorStoreMessage(int status) {
    lua_State* L = m_lua_state;
    if (status && !lua_isnil(L, -1))
    {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error object is not a string)";
        m_error_message = std::string{msg};
        lua_pop(L, 1);
    }
}

Interpreter::Result LuaInterpreter::Execute(const char *line) {
    lua_State* L = m_lua_state;
    size_t len = strlen(line);

    /* try to load the string as an expression */
    int status = yield_expr(L, m_lua_language, line, len);

    if (status != 0) {
        status = loadbuffer_by_language(L, m_lua_language, line, len, "=<user input>");
        if (incomplete(L, status))
            return Result::kIncompleteInput;
    }

    if (status == 0)
    {
        status = docall(L, 0, 0);
        LuaErrorStoreMessage(status);
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

    LuaErrorStoreMessage(status);
    return (status == 0 ? Result::kSuccess : Result::kError);
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

void LuaInterpreter::Interrupt() {
    lua_sethook(m_lua_state, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}
