
/* lua-utils.h
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

#ifndef LUA_UTILS_H
#define LUA_UTILS_H

#include "defs.h"
#include <lua.h>

extern void
mlua_openlibs (lua_State *L);

extern int
mlua_get_property (lua_State *L, const struct luaL_Reg *p, bool use_cache);

extern const struct luaL_Reg *
mlua_find_method (const struct luaL_Reg *p, const char *key);

extern void
mlua_null_cache (lua_State *L, int index);

extern void
mlua_check_field_type (lua_State *L, int index, const char *key, int type,
		       const char *error_msg);

extern int
mlua_index_with_properties (lua_State *L, const struct luaL_Reg *properties,
			    bool use_cache);

extern const char * mlua_named_optstring (lua_State *L, int index, 
					  const char *key, 
					  const char * default_value);

extern const char * mlua_named_string (lua_State *L, int index,
				       const char *key);

extern lua_Number   mlua_named_optnumber (lua_State *L, int index, 
					  const char *key, 
					  lua_Number default_value);

extern lua_Number   mlua_named_number    (lua_State *L, int index, 
					  const char *key);

#endif
