
/* lua-gsl.c
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

#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>

#include "lua-gsl.h"
#include "gs-types.h"
#include "lua-utils.h"
#include "fatal.h"

struct gsl_shell_state* global_state;

void
gsl_shell_open (struct gsl_shell_state *gs)
{
  gs->L = lua_open();  /* create state */

  if (unlikely(gs->L == NULL))
    fatal_exception("cannot create state: not enough memory");

  global_state = gs;
}

void
gsl_shell_init (struct gsl_shell_state *gs)
{
  pthread_mutex_init (&gs->exec_mutex, NULL);
  pthread_mutex_init (&gs->shutdown_mutex, NULL);
  gs->is_shutting_down = 0;
  gs->L = NULL;
}

void
gsl_shell_free (struct gsl_shell_state *gs)
{
  pthread_mutex_destroy (&gs->exec_mutex);
  pthread_mutex_destroy (&gs->shutdown_mutex);
}

int
luaopen_gsl (lua_State *L)
{
  gsl_set_error_handler_off ();

  lua_pushcfunction (L, gs_type_string);
  lua_setfield (L, LUA_REGISTRYINDEX, "__gsl_type");

  return 0;
}
