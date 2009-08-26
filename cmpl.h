
/* cmpl.h
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

#ifndef CMPL_VECTOR_H
#define CMPL_VECTOR_H

#include <math.h>
#include <gsl/gsl_vector_complex_double.h>

#ifdef __cplusplus
#define complex _Complex
#else
#include <complex.h>
#endif /* C++ */

#include "defs.h"

__BEGIN_DECLS

#define CSQABS(z) (creal(z)*creal(z) + cimag(z)*cimag(z))

typedef double complex cmpl;

extern void vec_complex_set (gsl_vector_complex *v, int i, cmpl val);
extern cmpl vec_complex_get (gsl_vector_complex *v, int i);

__END_DECLS

#endif
