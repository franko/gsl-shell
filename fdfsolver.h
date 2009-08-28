
#ifndef FDFSOLVER_H
#define FDFSOLVER_H

#include <lua.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit_nlin.h>

#include "defs.h"

struct fit_data {
  lua_State *L;
  gsl_vector *x;
  gsl_vector *j_raw;
  size_t n;
};

struct fdfsolver {
  gsl_multifit_fdfsolver *base;
  gsl_multifit_function_fdf fdf[1];
  struct fit_data fit_data[1];
  int base_type;
};

extern int                fdfsolver_dealloc    (lua_State *L);
extern struct fdfsolver * check_fdfsolver      (lua_State *L, int index);
extern struct fdfsolver * push_new_fdfsolver   (lua_State *L, 
						size_t n, size_t p);

extern char const * const fdfsolver_mt_name;
extern const struct luaL_Reg fdfsolver_methods[];

#endif
