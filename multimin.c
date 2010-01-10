
/* multimin.c
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

#include "multimin.h"

static const struct luaL_Reg multimin_functions[] = {
  {"fminimizer",    fmultimin_new},
  {"fdfminimizer",  fdfmultimin_new},
  {NULL, NULL}
};


void
multimin_register (lua_State *L)
{
  luaL_newmetatable (L, fmultimin_mt_name);
  luaL_register (L, NULL, fmultimin_methods);
  lua_pop (L, 1);

  luaL_newmetatable (L, fdfmultimin_mt_name);
  luaL_register (L, NULL, fdfmultimin_methods);
  lua_pop (L, 1);

  luaL_register (L, NULL, multimin_functions);
}
