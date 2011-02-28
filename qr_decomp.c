
#include <lua.h>
#include <lauxlib.h>

#include <gsl/gsl_linalg.h>

#include "gs-types.h"
#include "matrix.h"
#include "lua-utils.h"

typedef struct {
  gsl_matrix *m;
  gsl_vector *tau;
} qr_decomp;

static int qr_decomp_new      (lua_State *L);
static int qr_decomp_free     (lua_State *L);
static int qr_decomp_index    (lua_State *L);
static int qr_decomp_solve    (lua_State *L);
static int qr_decomp_lssolve  (lua_State *L);
static int qr_decomp_unpack   (lua_State *L);

const struct luaL_Reg qr_decomp_metatable[] = {
  {"__gc",          qr_decomp_free},
  {"__index",       qr_decomp_index},
  {NULL, NULL}
};

static const struct luaL_Reg qr_decomp_methods[] = {
  {"solve",         qr_decomp_solve},
  {"lssolve",       qr_decomp_lssolve},
  {"unpack",        qr_decomp_unpack},
  {NULL, NULL}
};


static const struct luaL_Reg qr_decomp_functions[] = {
  {"QR",         qr_decomp_new},
  {NULL, NULL}
};

int
qr_decomp_new (lua_State *L)
{
  int status;
  gsl_matrix *a = matrix_check (L, 1);
  size_t m = a->size1, n = a->size2;
  qr_decomp *qr = gs_new_object (sizeof(qr_decomp), L, GS_QR_DECOMP);
  size_t tn = (m <= n ? m : n);
  
  qr->m = gsl_matrix_alloc (m, n);
  gsl_matrix_memcpy (qr->m, a);

  qr->tau = gsl_vector_alloc (tn);
  if (qr->tau == NULL)
    return luaL_error (L, "out of memory");

  status = gsl_linalg_QR_decomp (qr->m, qr->tau);

  if (status != GSL_SUCCESS)
    {
      return luaL_error (L, "error during QR decomposition: %s", 
			 gsl_strerror (status));
    }

  lua_newtable (L);
  lua_setfenv (L, -2);

  return 1;
}

int
qr_decomp_free (lua_State *L)
{
  qr_decomp *qr = gs_check_userdata (L, 1, GS_QR_DECOMP);
  gsl_vector_free (qr->tau);
  gsl_matrix_free (qr->m);
  return 0;
}

int
qr_decomp_unpack (lua_State *L)
{
  qr_decomp *qr = gs_check_userdata (L, 1, GS_QR_DECOMP);
  size_t m = qr->m->size1, n = qr->m->size2;
  gsl_matrix *q = matrix_push_raw (L, m, m);
  gsl_matrix *r = matrix_push_raw (L, m, n);

  gsl_linalg_QR_unpack (qr->m, qr->tau, q, r);

  return 2;
}

int
qr_decomp_solve (lua_State *L)
{
  qr_decomp *qr = gs_check_userdata (L, 1, GS_QR_DECOMP);
  size_t m = qr->m->size1, n = qr->m->size2;
  gsl_matrix *b = matrix_check (L, 2);
  gsl_vector_view b1 = gsl_matrix_column (b, 0);

  if (m != n)
    return luaL_error (L, "\"solve\" requires a square matrix");

  if (b->size1 != n || b->size2 != 1)
    return luaL_error (L, "matrix dimensions mismatch");

  {
    int status;
    gsl_matrix *x = matrix_push_raw (L, n, 1);
    gsl_vector_view x1 = gsl_matrix_column (x, 0);
    status = gsl_linalg_QR_solve (qr->m, qr->tau, &b1.vector, &x1.vector);
    if (status != GSL_SUCCESS)
      {
	return luaL_error (L, "cannot solve the system: %s",
			   gsl_strerror (status));
      }
  }

  return 1;
}

int
qr_decomp_lssolve (lua_State *L)
{
  qr_decomp *qr = gs_check_userdata (L, 1, GS_QR_DECOMP);
  size_t m = qr->m->size1, n = qr->m->size2;
  gsl_matrix *b = matrix_check (L, 2);
  gsl_vector_view b1 = gsl_matrix_column (b, 0);

  if (m <= n)
    return luaL_error (L, "too few rows");

  if (b->size1 != m || b->size2 != 1)
    return luaL_error (L, "matrix dimensions mismatch");

  {
    int status;
    gsl_matrix *x = matrix_push_raw (L, n, 1);
    gsl_matrix *r = matrix_push_raw (L, m, 1);
    gsl_vector_view x1 = gsl_matrix_column (x, 0);
    gsl_vector_view r1 = gsl_matrix_column (r, 0);

    status = gsl_linalg_QR_lssolve (qr->m, qr->tau, &b1.vector, &x1.vector,
				    &r1.vector);

    if (status != GSL_SUCCESS)
      {
	return luaL_error (L, "cannot solve the system: %s",
			   gsl_strerror (status));
      }
  }

  return 2;
}

int
qr_decomp_index (lua_State *L)
{
  return mlua_index_methods (L, qr_decomp_methods);
}

void
qr_decomp_register (lua_State *L)
{
  luaL_newmetatable (L, GS_METATABLE(GS_QR_DECOMP));
  luaL_register (L, NULL, qr_decomp_metatable);
  lua_pop (L, 1);

  luaL_register (L, NULL, qr_decomp_functions);
}
