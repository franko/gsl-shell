
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

#include <gsl/gsl_errno.h>

#include "lua-gsl.h"

#include "gdt/gdt_table.h"

/* used to force the linker to link the gdt library. Otherwise it
 * would be discarded as there are no other references to its functions. */
extern gdt_table *(*_gdt_ref)(int nb_rows, int nb_columns, int nb_rows_alloc);
gdt_table *(*_gdt_ref)(int nb_rows, int nb_columns, int nb_rows_alloc) = gdt_table_new;

int
luaopen_gsl (lua_State *L)
{
    gsl_set_error_handler_off ();
    return 0;
}
