
typedef struct {
  TYPE(gsl_matrix) *m;
  gsl_permutation *p;
  int signum;
} TYPE(lu_decomp);

static int FUNCTION(lu_decomp, free)     (lua_State *L);
static int FUNCTION(lu_decomp, index)    (lua_State *L);
static int FUNCTION(lu_decomp, solve)    (lua_State *L);
static int FUNCTION(lu_decomp, det)      (lua_State *L);
static int FUNCTION(lu_decomp, L)        (lua_State *L);
static int FUNCTION(lu_decomp, U)        (lua_State *L);
static int FUNCTION(lu_decomp, P)        (lua_State *L);
static int FUNCTION(lu_decomp, signum)   (lua_State *L);

const struct luaL_Reg FUNCTION (lu_decomp, metatable)[] = {
  {"__gc",          FUNCTION(lu_decomp, free)},
  {"__index",       FUNCTION(lu_decomp, index)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION (lu_decomp, methods)[] = {
  {"solve",         FUNCTION(lu_decomp, solve)},
  {"det",           FUNCTION(lu_decomp, det)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION(lu_decomp, properties)[] = {
  {"L",             FUNCTION(lu_decomp, L)},
  {"U",             FUNCTION(lu_decomp, U)},
  {"P",             FUNCTION(lu_decomp, P)},
  {"signum",        FUNCTION(lu_decomp, signum)},
  {NULL, NULL}
};

int
FUNCTION(lu_decomp, raw) (lua_State *L, size_t n, TYPE(gsl_matrix) *m)
{
  int status;
  TYPE(lu_decomp) *lu = gs_new_object (sizeof(TYPE(lu_decomp)), L,
				       GS_TYPE(LU_DECOMP));
  
  lu->m = FUNCTION(gsl_matrix, alloc) (n, n);
  FUNCTION(gsl_matrix, memcpy) (lu->m, m);

  lu->p = gsl_permutation_alloc (n);
  if (lu->p == NULL)
    return luaL_error (L, "out of memory");

  status = FUNCTION(gsl_linalg, LU_decomp) (lu->m, lu->p, &lu->signum);

  if (status != GSL_SUCCESS)
    {
      return luaL_error (L, "error during LU decomposition: %s", 
			 gsl_strerror (status));
    }

  lua_newtable (L);
  lua_setfenv (L, -2);

  return 1;
}

int
FUNCTION(lu_decomp, free) (lua_State *L)
{
  TYPE(lu_decomp) *lu = gs_check_userdata (L, 1, GS_TYPE(LU_DECOMP));
  gsl_permutation_free (lu->p);
  FUNCTION(gsl_matrix, free) (lu->m);
  return 0;
}

int
FUNCTION(lu_decomp, L) (lua_State *L)
{
  TYPE(lu_decomp) *lu = gs_check_userdata (L, 1, GS_TYPE(LU_DECOMP));
  size_t n = lu->m->size1;
  TYPE(gsl_matrix) *r = FUNCTION (matrix, push_raw) (L, n, n);
  LUA_TYPE *rp = (LUA_TYPE *) r->data, *mp = (LUA_TYPE *) lu->m->data;
  int i;

  for (i = 0; i < n; i++)
    {
      LUA_TYPE *lm1 = mp + i, *lm2 = mp + n;
      for (; mp < lm1; rp++, mp++)
	*rp = *mp;

      *(rp++) = (LUA_TYPE) 1;
      mp++;

      for (; mp < lm2; rp++, mp++)
	*rp = (LUA_TYPE) 0;
    }
 
  return 1;
}


int
FUNCTION(lu_decomp, U) (lua_State *L)
{
  TYPE(lu_decomp) *lu = gs_check_userdata (L, 1, GS_TYPE(LU_DECOMP));
  size_t n = lu->m->size1;
  TYPE(gsl_matrix) *r = FUNCTION (matrix, push_raw) (L, n, n);
  LUA_TYPE *rp = (LUA_TYPE *) r->data, *mp = (LUA_TYPE *) lu->m->data;
  int i;

  for (i = 0; i < n; i++)
    {
      LUA_TYPE *lm1 = mp + i, *lm2 = mp + n;
      for (; mp < lm1; rp++, mp++)
	*rp = (LUA_TYPE) 0;

      *(rp++) = *(mp++);

      for (; mp < lm2; rp++, mp++)
	*rp = *mp;
    }
 
  return 1;
}


int
FUNCTION(lu_decomp, P) (lua_State *L)
{
  TYPE(lu_decomp) *lu = gs_check_userdata (L, 1, GS_TYPE(LU_DECOMP));
  size_t n = lu->m->size1;
  gsl_matrix *pm = matrix_push (L, n, n);
  double *ptr = pm->data;
  gsl_permutation *p = lu->p;
  int j;

  for (j = 0; j < n; j++)
    {
      int pj = p->data[j];
      ptr[j * n + pj] = 1.0;
    }
 
  return 1;
}

int
FUNCTION(lu_decomp, signum) (lua_State *L)
{
  TYPE(lu_decomp) *lu = gs_check_userdata (L, 1, GS_TYPE(LU_DECOMP));
  lua_pushnumber (L, lu->signum);
  return 1;
}

int
FUNCTION(lu_decomp, solve) (lua_State *L)
{
  TYPE(lu_decomp) *lu = gs_check_userdata (L, 1, GS_TYPE(LU_DECOMP));
  TYPE(gsl_matrix) *b = FUNCTION(matrix, check) (L, 2);
  VIEW(gsl_vector) b1 = FUNCTION(gsl_matrix, column) (b, 0);
  size_t n = lu->m->size1;

  if (b->size1 != n || b->size2 != 1)
    return gs_type_error (L, 2, "column matrix");

  {
    TYPE(gsl_matrix) *x = FUNCTION(matrix, push_raw) (L, n, 1);
    VIEW(gsl_vector) x1 = FUNCTION(gsl_matrix, column) (x, 0);
    FUNCTION (gsl_linalg, LU_solve) (lu->m, lu->p, &b1.vector, &x1.vector);
  }

  return 1;
}

int
FUNCTION(lu_decomp, det) (lua_State *L)
{
  TYPE(lu_decomp) *lu = gs_check_userdata (L, 1, GS_TYPE(LU_DECOMP));
  BASE d = FUNCTION (gsl_linalg, LU_det) (lu->m, lu->signum);
  LUA_TYPE *zp = (LUA_TYPE *) &d;
  LUA_FUNCTION(push) (L, *zp);
  return 1;
}

int
FUNCTION(lu_decomp, index) (lua_State *L)
{
  return mlua_index_with_properties (L,
				     FUNCTION(lu_decomp, properties), 
				     FUNCTION(lu_decomp, methods), 
				     true);
}
