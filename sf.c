
/* sf.c
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

#include <string.h>
#include <math.h>
#include <gsl/gsl_sf.h>

#include <lua.h>
#include <lauxlib.h>

#include "defs.h"

static gsl_mode_t
gsl_mode_from_string (const char *s)
{
  if (s == NULL)
    return GSL_PREC_DOUBLE;

  if (strcmp (s, "double") == 0)
    return GSL_PREC_DOUBLE;
  else if (strcmp (s, "single") == 0)
    return GSL_PREC_SINGLE;

  return GSL_PREC_APPROX;
}

static unsigned int
my_checkuint (lua_State *L, int index)
{
  int i = luaL_checkinteger (L, index);
  if (i < 0)
    {
      luaL_error (L, "non-negative integer expected");
      return 0;
    }

  return (unsigned int) i;
}

#include "sf_define.h"

#include "sf_declare.h"
#include "sf_gener.c"
#include "sf_implement_off.h"

static const struct luaL_Reg sf_functions[] = {
#include "sf_methods.h"
#include "sf_gener.c"
#include "sf_implement_off.h"
  {NULL, NULL}
};

#ifdef SF_RETURN_ERROR
#define SF_RETURN(p) \
  lua_pushnumber (L, (p)->val); \
  lua_pushnumber (L, (p)->err); \
  return 2
#else
#define SF_RETURN(p) \
  lua_pushnumber (L, (p)->val); \
  return 1
#endif

#include "sf_implement.h"
#include "sf_gener.c"
#include "sf_implement_off.h"

void
sf_register (lua_State *L)
{
  luaL_register (L, "sf", sf_functions);
  lua_pop(L, 1);
}
