
/* cmpl.c
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

#include <assert.h>
#include "common.h"
#include "cmpl.h"

void
vec_complex_set (gsl_vector_complex *v, int i, cmpl val)
{
  assert (i < v->size);
  *(cmpl *) (v->data + 2 * i * v->stride) = val;
}

cmpl
vec_complex_get (gsl_vector_complex *v, int i)
{
  assert (i < v->size);
  return *(cmpl *) (v->data + 2 * i * v->stride);
}
