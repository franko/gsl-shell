#ifndef RANDOM_H
#define RANDOM_H

#include <lua.h>
#include <gsl/gsl_rng.h>
#include "defs.h"

struct boxed_rng {
  gsl_rng *rng;
  unsigned long int min, max;
};

extern void random_register (lua_State *L);

extern gsl_rng *push_rng (lua_State *L, const gsl_rng_type *T);

#endif
