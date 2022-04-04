
/* lua-utils.c
 *
 * Copyright (C) 2009 Francesco Abbate
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
 */

#include <lua.h>
#include <lauxlib.h>
#include <string.h>
#include <stdio.h>
#include "lua-utils.h"
#include "gs-types.h"

lua_Number
mlua_named_optnumber (lua_State *L, int index, const char *key,
		      lua_Number default_value)
{
  lua_Number r;
  lua_getfield (L, index, key);
  r = luaL_optnumber (L, -1, default_value);
  lua_pop (L, 1);
  return r;
}

const char *
mlua_named_optstring (lua_State *L, int index, const char *key,
		      const char * default_value)
{
  const char * r;
  lua_getfield (L, index, key);
  r = luaL_optstring (L, -1, default_value);
  lua_pop (L, 1);
  return r;
}

lua_Number
mlua_named_number (lua_State *L, int index, const char *key)
{
  lua_Number r;
  lua_getfield (L, index, key);
  if (! lua_isnumber (L, -1))
    luaL_error (L, "number expected");
  r = lua_tonumber (L, -1);
  lua_pop (L, 1);
  return r;
}

const char *
mlua_named_string (lua_State *L, int index, const char *key)
{
  const char * r;
  lua_getfield (L, index, key);
  if (! lua_isstring (L, -1))
    luaL_error (L, "string expected");
  r = lua_tostring (L, -1);
  lua_pop (L, 1);
  return r;
}

int
error_return(lua_State* L, const char* error_msg)
{
    lua_pushstring(L, error_msg);
    return (-1);
}

int
type_error_return(lua_State* L, int narg, const char* req_type)
{
    const char *actual_type = full_type_name(L, narg);
    lua_pushfstring(L, "bad argument #%d (expected %s, got %s)", narg, req_type, actual_type);
    return (-1);
}
