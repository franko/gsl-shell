
/* common.h
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

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <math.h>
#include "defs.h"

__BEGIN_DECLS

#define DEGREE(d) ((d) * M_PI / 180.0)
#define SQR(x) ((x) * (x))

#ifdef DEBUG_MEM
#define emalloc(n) malloc((size_t) (n))
#define erealloc(x,n) realloc(x,(size_t) (n))
#else
#define emalloc(n) erealloc(NULL, n);
extern void *   erealloc                (void *p, int n);
#endif

#ifdef WIN32
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

struct generic_array {
  size_t number;
  size_t alloc;
  void *heap;
};

extern void   generic_array_check_alloc   (struct generic_array *s, int index,
					   size_t data_size);
extern void * generic_array_new           (size_t data_size);
extern void   generic_array_free          (struct generic_array *r);

#define ARRAY_CHECK_ALLOC(s,dtype,idx) generic_array_check_alloc((struct generic_array *) (s),idx,sizeof(dtype))
#define ARRAY_NEW(dtype) generic_array_new(sizeof(dtype))
#define ARRAY_FREE(s) generic_array_free(s)

__END_DECLS

#endif
