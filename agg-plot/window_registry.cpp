
#include "lua-defs.h"
#include "window_registry.h"

static char const * const refs_tname = "GSL.oirfs.wp";
static char const * const registry_tname = "GSL.reg.wins";

void
window_registry_prepare (lua_State *L)
{
    lua_newtable (L);
    lua_setfield (L, LUA_REGISTRYINDEX, registry_tname);

    lua_newtable (L);
    lua_setfield (L, LUA_REGISTRYINDEX, refs_tname);
}

int
window_index_add(lua_State *L, int index)
{
    INDEX_SET_ABS(L, index);

    lua_getfield (L, LUA_REGISTRYINDEX, registry_tname);

    int n = lua_objlen (L, -1);

    lua_pushvalue (L, index);
    lua_rawseti (L, -2, n+1);
    lua_pop (L, 1);

    return n+1;
}

void
window_index_get (lua_State *L, int id)
{
    lua_getfield (L, LUA_REGISTRYINDEX, registry_tname);
    lua_rawgeti (L, -1, id);
    lua_remove (L, -2);
}

void
window_index_remove (lua_State *L, int id)
{
    lua_getfield (L, LUA_REGISTRYINDEX, registry_tname);
    lua_getfield (L, LUA_REGISTRYINDEX, refs_tname);
    lua_rawgeti (L, -2, id);
    if (! lua_isnil (L, -1))
    {
        lua_pushnil (L);
        lua_rawset (L, -3);
        lua_pop (L, 1);
    }
    else
    {
        lua_pop (L, 2);
    }

    lua_pushnil (L);
    lua_rawseti (L, -2, id);
    lua_pop (L, 1);
}

void
window_index_apply_all (lua_State *L, lua_CFunction f)
{
    lua_getfield (L, LUA_REGISTRYINDEX, registry_tname);

    lua_pushnil (L);  /* first key */
    while (lua_next(L, -2) != 0)
    {
        lua_pushcfunction (L, f);
        lua_insert (L, -2);
        lua_call (L, 1, 0);
    }

    lua_pop (L, 1);
}

int
window_index_count (lua_State *L)
{
    int count = 0;

    lua_getfield (L, LUA_REGISTRYINDEX, registry_tname);

    lua_pushnil (L);  /* first key */
    while (lua_next(L, -2) != 0)
    {
        lua_pop (L, 1);
        count ++;
    }

    lua_pop (L, 1);

    return count;
}

/* A C function should be given in the top of the stack. The function
   take a window as argument and return a boolean. If it does returns
   true the windows will be removed. */
void
window_index_remove_fun(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, refs_tname);
    lua_getfield(L, LUA_REGISTRYINDEX, registry_tname);
    int n = lua_objlen(L, -1);
    for (int i = 1; i <= n; i++) {
        lua_rawgeti(L, -1, i); /* Retrieve the window corresponding to index i. */
        lua_pushvalue(L, -4); /* Push the C function. */
        lua_pushvalue(L, -2); /* Push the Window, can be nil. */
        lua_call(L, 1, 1);
        int remove = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (remove) {
            lua_pushnil(L);
            lua_rawset(L, -4); /* Set the reference to Window to nil. */
            lua_pushnil(L);
            lua_rawseti(L, -2, i); /* Eliminate the windows from the list. */
        } else {
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 3); /* Pop the reference tables, the windows list and the function. */
}

void
window_refs_add (lua_State *L, int index, int key_index, int value_index)
{
    INDEX_SET_ABS_2(L, key_index, value_index);

    lua_getfield (L, LUA_REGISTRYINDEX, refs_tname);
    lua_pushvalue (L, key_index);
    lua_rawget (L, -2);

    if (lua_isnil (L, -1))
    {
        lua_pop (L, 1);
        lua_newtable (L);
        lua_pushvalue (L, key_index);
        lua_pushvalue (L, -2);
        lua_rawset (L, -4);
    }

    lua_pushvalue (L, value_index);
    lua_rawseti (L, -2, index);

    lua_pop (L, 2);
}

void
window_refs_remove (lua_State *L, int index, int key_index)
{
    INDEX_SET_ABS(L, key_index);

    lua_getfield (L, LUA_REGISTRYINDEX, refs_tname);
    lua_pushvalue (L, key_index);
    lua_rawget (L, -2);

    if (! lua_isnil (L, -1))
    {
        lua_pushnil (L);
        lua_rawseti (L, -2, index);
    }

    lua_pop (L, 2);
}

void
window_refs_lookup_apply (lua_State *L, int value_index, lua_CFunction func)
{
    INDEX_SET_ABS(L, value_index);

    lua_getfield (L, LUA_REGISTRYINDEX, refs_tname);
    lua_pushnil (L);

    while (lua_next (L, -2) != 0)
    {
        lua_pushnil (L);

        while (lua_next (L, -2) != 0)
        {
            if (lua_rawequal (L, -1, value_index))
            {
                lua_pushcfunction (L, func);
                lua_pushvalue (L, -5);
                lua_pushvalue (L, -4);
                lua_call (L, 2, 0);
            }
            lua_pop (L, 1);
        }

        lua_pop (L, 1);
    }

    lua_pop (L, 1);
}
