
/* common.c
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
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"

#ifndef DEBUG_MEM
void *
erealloc (void *p, int n)
{
  void *newp;

  assert (n >= 0);

  newp = realloc (p, n);

  if (newp == NULL)
    {
      fprintf (stderr, "Couldn't allocate memory\n");
      fflush (stderr);
      exit (1);
    }
  return newp;
}
#endif

void
generic_array_check_alloc (struct generic_array *s, int idx,
			   size_t data_size)
{
  void *oldptr;

  assert (idx >= 0);

  if ((size_t)idx < s->alloc)
    return;

  oldptr = s->heap;
  do
    s->alloc *= 2;
  while ((size_t)idx >= s->alloc);

  s->heap = emalloc (s->alloc * data_size);
  memcpy (s->heap, oldptr, s->number * data_size);
  free (oldptr);
}

void *
generic_array_new (size_t data_size)
{
  struct generic_array *r;
  const size_t nb_init = 8;

  r = emalloc (sizeof(struct generic_array));

  r->number = 0;
  r->alloc = nb_init;
  r->heap = emalloc (r->alloc * data_size);

  return (void *) r;
}

void
generic_array_free (struct generic_array *r)
{
  free (r->heap);
  free (r);
}
