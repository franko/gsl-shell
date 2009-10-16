
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

char const * const CACHE_FIELD_NAME = "__cache";

const struct luaL_Reg *
mlua_find_method (const struct luaL_Reg *p, const char *key)
{
  for (/* */; p->name; p++)
    {
      if (strcmp (p->name, key) == 0)
	return p;
    }
  return NULL;
}

int
mlua_get_property (lua_State *L, const struct luaL_Reg *p, bool use_cache)
{
  int rval;
  bool cache_is_new = false;

  if (! use_cache)
    return p->func (L);

  lua_getfield (L, 1, CACHE_FIELD_NAME);
  if (lua_isnil (L, -1))
    {
      lua_pop (L, 1);
      lua_newtable (L);
      lua_pushvalue (L, -1);
      lua_setfield (L, 1, CACHE_FIELD_NAME);
      cache_is_new = true;
    }

  if (! cache_is_new)
    {
      lua_getfield (L, -1, p->name);
      if (! lua_isnil (L, -1))
	return 1;
      lua_pop (L, 1);
    }

  rval = p->func (L);
  if (rval == 1)
    {
      lua_pushvalue (L, -1);
      lua_setfield (L, -3, p->name);
      return 1;
    }
  return rval;
}

void
mlua_null_cache (lua_State *L, int index)
{
  lua_pushnil (L);
  lua_setfield (L, index, CACHE_FIELD_NAME);
}

int
mlua_index_with_properties (lua_State *L, const struct luaL_Reg *properties,
			    bool use_cache)
{
  char const * key;
  const struct luaL_Reg *reg;

  key = lua_tostring (L, 2);
  if (key == NULL)
    return 0;

  reg = mlua_find_method (properties, key);
  if (reg)
    {
      return mlua_get_property (L, reg, use_cache);
    }

  lua_getmetatable (L, 1);
  lua_replace (L, 1);
  lua_gettable (L, 1);
  return 1;
}

void
mlua_check_field_type (lua_State *L, int index, const char *key, int type,
		       const char *error_msg)
{
  lua_getfield (L, index, key);
  if (lua_type (L, -1) != type)
    {
      if (error_msg)
	luaL_error (L, "field \"%s\", ", key, error_msg);
      else
	luaL_error (L, "field \"%s\" should be an %s", key, 
		    lua_typename (L, type));
    }
  lua_pop (L, 1);
}

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
