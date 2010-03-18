
#include <lua.h>
#include <lauxlib.h>
#include <assert.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#include "matrix.h"
#include "cmatrix.h"

struct eigen_symm_cache {
  size_t n;
  gsl_eigen_symm_workspace *ws;
  gsl_eigen_symmv_workspace *vws;
  gsl_vector *diag;
};

struct eigen_cache {
  struct eigen_symm_cache symm[1];
};

#define EIGEN_CACHE_MT_NAME "GSL.eigcache"

static int           eigen_symm                 (lua_State *L);
static int           eigen_cache_free           (lua_State *L);

static void          eigen_push_cache           (lua_State *L);

static struct eigen_cache * eigen_cache_symm_set (lua_State *L, size_t n);

static const struct luaL_Reg eigen_cache_methods[] = {
  {"__gc",       eigen_cache_free},
  {NULL, NULL}
};  

static const struct luaL_Reg eigen_functions[] = {
  {"eigsymm",        eigen_symm},
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

int
eigen_cache_free (lua_State *L)
{
  struct eigen_cache *cache = luaL_checkudata (L, 1, EIGEN_CACHE_MT_NAME);

  if (cache->symm->ws)
    free_symm_cache (cache->symm);

  return 0;
}

void
eigen_push_cache (lua_State *L)
{
  struct eigen_cache *cache;

  cache = lua_newuserdata (L, sizeof(struct eigen_cache));

  luaL_getmetatable (L, EIGEN_CACHE_MT_NAME);
  lua_setmetatable (L, -2);

  cache->symm->ws   = NULL;
  cache->symm->vws  = NULL; 
  cache->symm->diag = NULL; 
  cache->symm->n    = 0;
}

struct eigen_cache *
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

  return cache;
}

int
eigen_symm (lua_State *L)
{
  gsl_matrix *m = matrix_check (L, 1);
  size_t i, j, n = m->size1;
  struct eigen_cache *cache;
  gsl_matrix *eval;
  gsl_vector *diag;
  gsl_vector_view eview;

  if (m->size1 != m->size2)
    return luaL_typerror (L, 1, "real symmetric matrix");

  cache = eigen_cache_symm_set (L, n);

  eval = matrix_push_raw (L, n, 1);
  eview = gsl_matrix_column (eval, 0);

  diag = cache->symm->diag;

  for (j = 0; j < n; j++)
    gsl_vector_set (diag, j, gsl_matrix_get (m, j, j));

  gsl_eigen_symm (m, &eview.vector, cache->symm->ws);

  for (j = 0; j < n; j++)
    gsl_matrix_set (m, j, j, gsl_vector_get (diag, j));

  for (i = 0; i < n; i++)
    for (j = 0; j < i; j++)
      gsl_matrix_set (m, i, j, gsl_matrix_get (m, j, i));

  return 1;
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
