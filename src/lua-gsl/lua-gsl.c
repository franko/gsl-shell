
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

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>

#include "lua-gsl.h"
#include "gs-types.h"
#include "lua-utils.h"
#include "fatal.h"

#include "gdt_table.h"

/* used to force the linker to link the gdt library. Otherwise it
 * would be discarded as there are no other references to its functions. */
extern gdt_table *(*_gdt_ref)(int nb_rows, int nb_columns, int nb_rows_alloc);
gdt_table *(*_gdt_ref)(int nb_rows, int nb_columns, int nb_rows_alloc) = gdt_table_new;

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

void run_start_script(lua_State *L) {
#ifdef _WIN32
#define PATHSEP_PATTERN "\\\\"
#define NONPATHSEP_PATTERN "[^\\\\]+"
#else
#define PATHSEP_PATTERN "/"
#define NONPATHSEP_PATTERN "[^/]+"
#endif

  const char *init_code = \
    "xpcall(function()\n"
    "  local exedir = EXEFILE:match('^(.*)" PATHSEP_PATTERN NONPATHSEP_PATTERN"$')\n"
    "  local prefix = exedir:match('^(.*)" PATHSEP_PATTERN "bin$')\n"
    "  dofile((prefix and prefix .. '/share/gsl-shell' or exedir .. '/lua') .. '/start.lua')\n"
    "end, function(err)\n"
    "  local error_dir\n"
    "  io.stdout:write('Error: '..tostring(err)..'\\n')\n"
    "  io.stdout:write(debug.traceback(nil, 4)..'\\n')\n"
    "  os.exit(1)\n"
    "end)\n";

  if (luaL_loadstring(L, init_code)) {
    fprintf(stderr, "internal error when starting the application\n");
    exit(1);
  }
  lua_pcall(L, 0, 0, 0);
}

int
luaopen_gsl (lua_State *L)
{
  gsl_set_error_handler_off ();

  lua_pushcfunction (L, gs_type_string);
  lua_setfield (L, LUA_REGISTRYINDEX, "__gsl_type");

  return 0;
}
