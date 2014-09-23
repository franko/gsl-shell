#include "lua-properties.h"

static void
register_cfuncs(lua_State* L, const luaL_Reg reg_table[])
{
  const luaL_Reg* reg;
  for (reg = reg_table; reg->name; reg++)
  {
    lua_pushstring(L, reg->name);
    lua_pushcfunction(L, reg->func);
    lua_rawset(L, -3);
  }
}

static int
properties_index (lua_State *L)
{
  lua_pushvalue(L, 2); /* push the key */
  lua_rawget(L, lua_upvalueindex(1)); /* lookup into methods' table (upvalue) */
  if (lua_isnil(L, -1)) /* if the method does not exists */
  {
    lua_pushvalue(L, 2); /* push the key */
    lua_rawget(L, lua_upvalueindex(2)); /* lookup into getters table (upvalue) */
    if (lua_isnil(L, -1)) /* if getter function does not exists */
      return 1; /* return nil */

    /* the getter exists: */
    lua_pushvalue(L, 1); /* push the object itself (plot) */
    lua_call(L, 1, 1); /* call the getter and return the result */
    return 1;
  }

  /* the method exists. We return it (a function). */
  return 1;
}

static int
properties_newindex (lua_State *L)
{
  lua_pushvalue(L, 2); /* push the key */
  lua_rawget(L, lua_upvalueindex(1)); /* lookup into setters table (upvalue) */
  if (!lua_isnil(L, -1)) /* if setter function does exists */
  {
    lua_pushvalue(L, 1); /* push the object itself (plot) */
    lua_pushvalue(L, 3); /* push the value for the setter */
    /* call the setter function with the plot and value as arguments */
    lua_call(L, 2, 0);
    return 0;
  }
  return 0;
}

/* Assume that we have on top of the stack a table and add
   a __index and __newindex function to access the methods and properties
  getters. Leave the table on top of the stack. */
void
register_properties_index(lua_State* L, const luaL_Reg methods[],
  const luaL_Reg getters[], const luaL_Reg setters[])
{
  lua_pushstring(L, "__index");
  lua_newtable(L); /* create a new table to hold the methods */
  register_cfuncs(L, methods); /* register all the methods into the table */
  lua_newtable(L); /* create a new table to hold the getters */
  register_cfuncs(L, getters); /* register all the getters into the table */
  /* create a closure with the methods and getters tables as upvalues */
  lua_pushcclosure(L, properties_index, 2);
  lua_rawset(L, -3); /* bind the newly created closure to __index */

  lua_pushstring(L, "__newindex");
  lua_newtable(L); /* create a new table to hold the setters */
  register_cfuncs(L, setters); /* register all the setters into the table */
  /* create a closure with the setters' table as upvalue */
  lua_pushcclosure(L, properties_newindex, 1);
  lua_rawset(L, -3); /* bind the newly created closure to __newindex */
}
