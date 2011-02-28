
#include <lua.h>
#include <lauxlib.h>

#include <gsl/gsl_linalg.h>

#include "gs-types.h"
#include "matrix.h"
#include "lua-utils.h"
#include "lu_decomp_imp.h"

#define BASE_DOUBLE
#include "template_matrix_on.h"
#include "lu_decomp_source.c"
#include "template_matrix_off.h"
