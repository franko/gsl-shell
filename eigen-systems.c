
#include <lua.h>
#include <lauxlib.h>
#include <assert.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_linalg.h>

#include "matrix.h"
#include "cmatrix.h"
#include "eigen-systems.h"

struct eigen_symm_cache {
  size_t n;
  gsl_eigen_symm_workspace *ws;
  gsl_eigen_symmv_workspace *vws;
  gsl_vector *diag;
};

struct eigen_herm_cache {
  size_t n;
  gsl_eigen_herm_workspace *ws;
  gsl_eigen_hermv_workspace *vws;
  gsl_vector_complex *diag;
};

struct eigen_nonsymm_cache {
  size_t n;
  gsl_eigen_nonsymm_workspace *ws;
  gsl_eigen_nonsymmv_workspace *vws;
  gsl_matrix *mcopy;
};

struct eigen_cache {
  struct eigen_symm_cache symm[1];
  struct eigen_herm_cache herm[1];
  struct eigen_nonsymm_cache nonsymm[1];
};

#define EIGEN_CACHE_MT_NAME "GSL.eigcache"

static int           eigen_symm                 (lua_State *L);
static int           eigen_symmv                (lua_State *L);
static int           eigen_herm                 (lua_State *L);
static int           eigen_hermv                (lua_State *L);
static int           eigen_nonsymm              (lua_State *L);
static int           eigen_nonsymmv             (lua_State *L);
static int           schur_decomp               (lua_State *L);
static int           eigen_cache_free           (lua_State *L);

static void          eigen_push_cache           (lua_State *L);

static struct eigen_symm_cache * eigen_cache_symm_set (lua_State *L, size_t n);
static struct eigen_herm_cache * eigen_cache_herm_set (lua_State *L, size_t n);
static struct eigen_nonsymm_cache * eigen_cache_nonsymm_set (lua_State *L, size_t n);

static const struct luaL_Reg eigen_cache_methods[] = {
  {"__gc",       eigen_cache_free},
  {NULL, NULL}
};  

static const struct luaL_Reg eigen_functions[] = {
  {"eigs",        eigen_symm},
  {"eigsv",       eigen_symmv},
  {"eigh",        eigen_herm},
  {"eighv",       eigen_hermv},
  {"eigns",       eigen_nonsymm},
  {"eignsv",      eigen_nonsymmv},
  {"schur",       schur_decomp},
  {NULL, NULL}
};  

static void
free_symm_cache (struct eigen_symm_cache *symm)
{
  gsl_eigen_symm_free  (symm->ws );
  gsl_eigen_symmv_free (symm->vws);
  gsl_vector_free      (symm->diag);
  symm->n = 0;
}

static void
free_herm_cache (struct eigen_herm_cache *herm)
{
  gsl_eigen_herm_free     (herm->ws );
  gsl_eigen_hermv_free    (herm->vws);
  gsl_vector_complex_free (herm->diag);
  herm->n = 0;
}

static void
free_nonsymm_cache (struct eigen_nonsymm_cache *nonsymm)
{
  gsl_eigen_nonsymm_free  (nonsymm->ws );
  gsl_eigen_nonsymmv_free (nonsymm->vws);
  gsl_matrix_free         (nonsymm->mcopy);
  nonsymm->n = 0;
}

int
eigen_cache_free (lua_State *L)
{
  struct eigen_cache *cache = luaL_checkudata (L, 1, EIGEN_CACHE_MT_NAME);

  if (cache->symm->ws)
    free_symm_cache (cache->symm);

  if (cache->herm->ws)
    free_herm_cache (cache->herm);

  if (cache->nonsymm->ws)
    free_nonsymm_cache (cache->nonsymm);

  return 0;
}

void
eigen_push_cache (lua_State *L)
{
  struct eigen_cache *cache;

  cache = lua_newuserdata (L, sizeof(struct eigen_cache));

  luaL_getmetatable (L, EIGEN_CACHE_MT_NAME);
  lua_setmetatable (L, -2);

  cache->symm->ws = NULL;
  cache->symm->n  = 0;

  cache->herm->ws = NULL;
  cache->herm->n  = 0;

  cache->nonsymm->ws = NULL;
  cache->nonsymm->n  = 0;
}

struct eigen_symm_cache *
eigen_cache_symm_set (lua_State *L, size_t n)
{
  struct eigen_cache *cache;

  lua_getfield(L, LUA_ENVIRONINDEX, "cache");
  cache = lua_touserdata (L, -1);
  lua_pop (L, 1);
  
  assert (cache != NULL);

  if (!cache->symm->ws || cache->symm->n != n)
    {
      if (cache->symm->ws)
	free_symm_cache (cache->symm);

      cache->symm->ws   = gsl_eigen_symm_alloc  (n);
      cache->symm->vws  = gsl_eigen_symmv_alloc (n);
      cache->symm->diag = gsl_vector_alloc (n);
      cache->symm->n = n;
    }

  return cache->symm;
}

struct eigen_herm_cache *
eigen_cache_herm_set (lua_State *L, size_t n)
{
  struct eigen_cache *cache;

  lua_getfield(L, LUA_ENVIRONINDEX, "cache");
  cache = lua_touserdata (L, -1);
  lua_pop (L, 1);
  
  assert (cache != NULL);

  if (!cache->herm->ws || cache->herm->n != n)
    {
      if (cache->herm->ws)
	free_herm_cache (cache->herm);

      cache->herm->ws   = gsl_eigen_herm_alloc  (n);
      cache->herm->vws  = gsl_eigen_hermv_alloc (n);
      cache->herm->diag = gsl_vector_complex_alloc (n);
      cache->herm->n = n;
    }

  return cache->herm;
}

struct eigen_nonsymm_cache *
eigen_cache_nonsymm_set (lua_State *L, size_t n)
{
  struct eigen_cache *cache;

  lua_getfield(L, LUA_ENVIRONINDEX, "cache");
  cache = lua_touserdata (L, -1);
  lua_pop (L, 1);
  
  assert (cache != NULL);

  if (!cache->nonsymm->ws || cache->nonsymm->n != n)
    {
      if (cache->nonsymm->ws)
	free_nonsymm_cache (cache->nonsymm);
      
      cache->nonsymm->ws    = gsl_eigen_nonsymm_alloc  (n);
      cache->nonsymm->vws   = gsl_eigen_nonsymmv_alloc (n);
      cache->nonsymm->mcopy = gsl_matrix_alloc (n, n);
      cache->nonsymm->n = n;
    }

  return cache->nonsymm;
}

int
eigen_symm_raw (lua_State *L, int compute_evecs)
{
  gsl_matrix *m = matrix_check (L, 1);
  size_t i, j, n = m->size1;
  struct eigen_symm_cache *cache;
  gsl_matrix *eval, *evec;
  gsl_vector *diag;
  gsl_vector_view eview;

  if (m->size1 != m->size2)
    return luaL_typerror (L, 1, "real symmetric matrix");

  cache = eigen_cache_symm_set (L, n);

  eval = matrix_push_raw (L, n, 1);
  eview = gsl_matrix_column (eval, 0);

  if (compute_evecs)
    evec = matrix_push_raw (L, n, n);

  diag = cache->diag;

  for (j = 0; j < n; j++)
    gsl_vector_set (diag, j, gsl_matrix_get (m, j, j));

  if (compute_evecs)
    gsl_eigen_symmv (m, &eview.vector, evec, cache->vws);
  else
    gsl_eigen_symm (m, &eview.vector, cache->ws);

  for (j = 0; j < n; j++)
    gsl_matrix_set (m, j, j, gsl_vector_get (diag, j));

  for (i = 0; i < n; i++)
    for (j = 0; j < i; j++)
      gsl_matrix_set (m, i, j, gsl_matrix_get (m, j, i));

  return (compute_evecs ? 2 : 1);
}

int
eigen_symm (lua_State *L)
{
  return eigen_symm_raw (L, 0);
}

int
eigen_symmv (lua_State *L)
{
  return eigen_symm_raw (L, 1);
}

static int
eigen_herm_raw (lua_State *L, int compute_evecs)
{
  gsl_matrix_complex *m = matrix_complex_check (L, 1);
  size_t i, j, n = m->size1;
  struct eigen_herm_cache *cache;
  gsl_matrix *eval;
  gsl_matrix_complex *evec;
  gsl_vector_complex *diag;
  gsl_vector_view eview;

  if (m->size1 != m->size2)
    return luaL_typerror (L, 1, "real hermitian matrix");

  cache = eigen_cache_herm_set (L, n);

  eval = matrix_push_raw (L, n, 1);
  eview = gsl_matrix_column (eval, 0);

  if (compute_evecs)
    evec = matrix_complex_push_raw (L, n, n);

  diag = cache->diag;

  for (j = 0; j < n; j++)
    gsl_vector_complex_set (diag, j, gsl_matrix_complex_get (m, j, j));

  if (compute_evecs)
    gsl_eigen_hermv (m, &eview.vector, evec, cache->vws);
  else
    gsl_eigen_herm (m, &eview.vector, cache->ws);

  for (j = 0; j < n; j++)
    gsl_matrix_complex_set (m, j, j, gsl_vector_complex_get (diag, j));

  for (i = 0; i < n; i++)
    for (j = 0; j < i; j++)
      {
	gsl_complex v = gsl_matrix_complex_get (m, j, i);
	gsl_complex vc = gsl_complex_conjugate (v);
	gsl_matrix_complex_set (m, i, j, vc);
      }

  return (compute_evecs ? 2 : 1);
}

int
eigen_herm (lua_State *L)
{
  return eigen_herm_raw (L, 0);
}

int
eigen_hermv (lua_State *L)
{
  return eigen_herm_raw (L, 1);
}

static int
eigen_nonsymm_raw (lua_State *L, int compute_evec)
{
  gsl_matrix *m = matrix_check (L, 1), *mcopy;
  size_t n = m->size1;
  struct eigen_nonsymm_cache *cache;
  gsl_matrix_complex *eval, *evec;
  gsl_vector_complex_view eview;
  int retnval = 1;
  int status;

  if (m->size1 != m->size2)
    return luaL_typerror (L, 1, "real matrix");

  cache = eigen_cache_nonsymm_set (L, n);

  eval = matrix_complex_push_raw (L, n, 1);
  eview = gsl_matrix_complex_column (eval, 0);

  mcopy = cache->mcopy;

  gsl_matrix_memcpy (mcopy, m);

  if (compute_evec)
    {
      evec = matrix_complex_push_raw (L, n, n);
      retnval ++;
      status = gsl_eigen_nonsymmv (mcopy, &eview.vector, evec, cache->vws);
    }
  else
    {
      gsl_eigen_nonsymm_params (0, 0, cache->ws);
      status = gsl_eigen_nonsymm (mcopy, &eview.vector, cache->ws);
    }


  if (status)
    {
      return luaL_error (L, "error during non-symmetric eigenvalues"
			 " determination: %s", 
			 gsl_strerror (status));
    }

  return retnval;
}

int
eigen_nonsymm (lua_State *L)
{
  return eigen_nonsymm_raw (L, 0);
}

int
eigen_nonsymmv (lua_State *L)
{
  return eigen_nonsymm_raw (L, 1);
}

int
schur_decomp (lua_State *L)
{
  gsl_matrix *m = matrix_check (L, 1);
  size_t n = m->size1;
  struct eigen_nonsymm_cache *cache;
  gsl_matrix_complex *eval, *evec;
  gsl_vector_complex_view eview;
  gsl_matrix *t, *z;
  int status;

  if (m->size1 != m->size2)
    return luaL_typerror (L, 1, "real matrix");

  cache = eigen_cache_nonsymm_set (L, n);

  eval = matrix_complex_push_raw (L, n, 1);
  eview = gsl_matrix_complex_column (eval, 0);

  evec = matrix_complex_push_raw (L, n, n);

  t = matrix_push_raw (L, n, n);
  gsl_matrix_memcpy (t, m);

  z = matrix_push_raw (L, n, n);

  status = gsl_eigen_nonsymmv_Z (t, &eview.vector, evec, z, cache->vws);

  if (status)
    {
      return luaL_error (L, "error during Schur decomposition: %s", 
			 gsl_strerror (status));
    }

  gsl_linalg_hessenberg_set_zero (t);

  return 2;
}

void
eigen_register (lua_State *L)
{
  luaL_newmetatable (L, EIGEN_CACHE_MT_NAME);
  luaL_register (L, NULL, eigen_cache_methods);
  lua_pop (L, 1);

  lua_newtable (L);
  eigen_push_cache (L);
  lua_setfield (L, -2, "cache");
  lua_replace (L, LUA_ENVIRONINDEX);

  luaL_register (L, NULL, eigen_functions);
}
