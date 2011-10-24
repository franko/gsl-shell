#ifndef LUA_RNG_H
#define LUA_RNG_H

#include <lua.h>
#include <gsl/gsl_rng.h>

extern void rng_register (lua_State *L);

struct lua_rng {
  gsl_rng *rng;
  unsigned long int min, max;
};

#endif
