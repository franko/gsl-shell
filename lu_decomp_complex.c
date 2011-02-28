
#include <lua.h>
#include <lauxlib.h>

#include <gsl/gsl_linalg.h>

#include "gs-types.h"
#include "matrix.h"
#include "cmatrix.h"
#include "lcomplex.h"
#include "lua-utils.h"
#include "lu_decomp_imp.h"

#define BASE_GSL_COMPLEX
#include "template_matrix_on.h"
#include "lu_decomp_source.c"
#include "template_matrix_off.h"
