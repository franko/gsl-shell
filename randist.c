
/* randist.c
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

#include "gs-types.h"
#include "lua-rng.h"
#include "randist.h"

typedef double (*gsl_ran_func_t)(const gsl_rng *, double);
typedef double (*gsl_ran_func_2p_t)(const gsl_rng *, double, double);

#define RAN_IMPLEMENT(name) \
int randist_ ## name (lua_State *L) \
  { \
    return randist_gener_raw (L, gsl_ran_ ## name); \
  }

#define RAN_IMPLEMENT_2P(name) \
int randist_ ## name (lua_State *L) \
  { \
    return randist_gener_raw_2p (L, gsl_ran_ ## name); \
  }

#define RAN_DECLARE(name) static int randist_ ## name (lua_State *L)
#define LUAREG_DECLARE(name) {#name, randist_ ## name}

RAN_DECLARE (gaussian);
RAN_DECLARE (exponential);
RAN_DECLARE (chisq);
RAN_DECLARE (laplace);
RAN_DECLARE (tdist);
RAN_DECLARE (cauchy);
RAN_DECLARE (poisson);
RAN_DECLARE (fdist);
RAN_DECLARE (gamma);
RAN_DECLARE (beta);
RAN_DECLARE (binomial);
RAN_DECLARE (gaussian_tail);
RAN_DECLARE (exppow);
RAN_DECLARE (rayleigh);
RAN_DECLARE (lognormal);
RAN_DECLARE (flat);

static const struct luaL_Reg randist_functions[] = {
  LUAREG_DECLARE (gaussian),
  LUAREG_DECLARE (exponential),
  LUAREG_DECLARE (chisq),
  LUAREG_DECLARE (laplace),
  LUAREG_DECLARE (tdist),
  LUAREG_DECLARE (poisson),
  LUAREG_DECLARE (fdist),
  LUAREG_DECLARE (gamma),
  LUAREG_DECLARE (beta),
  LUAREG_DECLARE (binomial),
  LUAREG_DECLARE (gaussian_tail),
  LUAREG_DECLARE (exppow),
  LUAREG_DECLARE (cauchy),
  LUAREG_DECLARE (rayleigh),
  LUAREG_DECLARE (lognormal),
  LUAREG_DECLARE (flat),
  {NULL, NULL}
};

static int
randist_gener_raw (lua_State *L, gsl_ran_func_t func)
{
  struct lua_rng *urng = gs_check_userdata (L, 1, GS_RNG);
  double param = luaL_optnumber (L, 2, 1.0);
  double v = func (urng->rng, param);
  lua_pushnumber (L, v);
  return 1;
}

static int
randist_gener_raw_2p (lua_State *L, gsl_ran_func_2p_t func)
{
  struct lua_rng *urng = gs_check_userdata (L, 1, GS_RNG);
  double p1 = luaL_checknumber (L, 2);
  double p2 = luaL_checknumber (L, 3);
  double v = func (urng->rng, p1, p2);
  lua_pushnumber (L, v);
  return 1;
}

RAN_IMPLEMENT(gaussian)
RAN_IMPLEMENT(exponential)
RAN_IMPLEMENT(chisq)
RAN_IMPLEMENT(laplace)
RAN_IMPLEMENT(tdist)
RAN_IMPLEMENT(cauchy)
RAN_IMPLEMENT(rayleigh)

RAN_IMPLEMENT_2P(fdist)
RAN_IMPLEMENT_2P(gamma)
RAN_IMPLEMENT_2P(beta)
RAN_IMPLEMENT_2P(gaussian_tail)
RAN_IMPLEMENT_2P(exppow)
RAN_IMPLEMENT_2P(lognormal)
RAN_IMPLEMENT_2P(flat)

int
randist_binomial (lua_State *L)
{
  struct lua_rng *urng = gs_check_userdata (L, 1, GS_RNG);
  double p1 = luaL_checknumber (L, 2);
  int p2 = luaL_checkinteger (L, 3);
  double v;

  if (p2 < 0)
    luaL_error (L, "parameter n cannot be negative for binomial distribution");

  v = gsl_ran_binomial (urng->rng, p1, (unsigned int) p2);
  lua_pushnumber (L, v);
  return 1;
}

static int
randist_poisson (lua_State *L)
{
  struct lua_rng *urng = gs_check_userdata (L, 1, GS_RNG);
  double param = luaL_optnumber (L, 2, 1.0);
  unsigned int v = gsl_ran_poisson (urng->rng, param);
  lua_pushnumber (L, (double) v);
  return 1;
}

void
randist_register (lua_State *L)
{
  luaL_register (L, "rnd", randist_functions);
  lua_pop(L, 1);
}
