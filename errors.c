
/* errors.c
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
#include <gsl/gsl_errno.h>

#include "errors.h"

static lua_State *sys_wide_lua_state = NULL;

/*
struct error_reg {
  int error_code;
  const char *id;
  const char *long_msg;
};

static const struct error_reg error_codes[] = {
  {GSL_EMAXITER, "maxiter"},
  {GSL_EROUND,   "failed"},
  {GSL_ESING,    "singular"},
  {GSL_EDIVERGE, "diverge"},
  {0, NULL}
};

static const char *
integ_get_error_msg (int code)
{
  const char *default_msg = "unknown";
  const struct error_reg *p;
  for (p = error_codes; p->msg; p++)
    {
      if (p->error_code == code)
	return p->msg;
    }
  return default_msg;
}
*/

void
my_default_handler (const char * reason,
		    const char * file,
		    int line,
		    int gsl_errno)
{
  luaL_error (sys_wide_lua_state, "GSL error: %s", reason);
}

void
set_gsl_error_handler (lua_State *L)
{
  gsl_error_handler_t *old_handler;
  sys_wide_lua_state = L;
  old_handler = gsl_set_error_handler (& my_default_handler);
}
