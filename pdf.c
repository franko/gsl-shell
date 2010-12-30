
/* pdf.c
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
#include <string.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>

#include "random.h"
#include "pdf.h"

#define CONCAT2x(a,b) a ## _ ## b 
#define CONCAT2(a,b) CONCAT2x(a,b)
#define QUOTE(x) #x

#define SUFFIX pdf
#define DECLINE(name) CONCAT2(name, SUFFIX)
#define INCLINE(name) CONCAT2(SUFFIX, name)
#define MY_GSL_FUNC(name) CONCAT2(gsl_ran, DECLINE(name))
#define MODULE_NAME "pdf"

#include "randist-source.c"
