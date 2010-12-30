
/* cdf.c
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
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_sf_erf.h>

#include "random.h"
#include "cdf.h"

#define CONCAT2x(a,b) a ## _ ## b 
#define CONCAT2(a,b) CONCAT2x(a,b)
#define QUOTE(x) #x

#define SUFFIX cdf
#define DECLINE(name) CONCAT2(name, SUFFIX)
#define INCLINE(name) CONCAT2(SUFFIX, name)
#define MY_GSL_FUNC(name) CONCAT2(gsl_cdf, CONCAT2 (name, P))
#define MODULE_NAME "cdf"

extern double gsl_cdf_gaussian_tail_P (double, double, double);

#include "randist-source.c"

double
gsl_cdf_gaussian_tail_P (double x, double a, double sigma)
{
  if (x < a)
    {
      return 0;
    }
  else
    {
      double N, p;
      double xf = sqrt (2.0) * sigma;

      double f = gsl_sf_erfc (a / xf);
      double p0 = gsl_sf_erf (a / xf);

      N = 0.5 * f;

      p = (1 / (2 * N)) * (gsl_sf_erf (x / xf) - p0);

      return p;
    }
}
