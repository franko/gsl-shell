
typedef double (*my_gsl_func_t)(double, double);
typedef double (*my_gsl_func_2p_t)(double, double, double);

#define MY_FUNC(name) CONCAT2 (my, DECLINE (name))
#define MY_DECLARE(name) static int MY_FUNC (name) (lua_State *L);
#define REG_MY_DECLARE(name) {#name, MY_FUNC (name)},

#define MY_FUNC_IMPLEMENT(name)		\
int MY_FUNC (name) (lua_State *L)	\
  {							\
    return DECLINE (gener_raw) (L, MY_GSL_FUNC (name));	\
  }

#define MY_FUNC_IMPLEMENT_2P(name)		\
int MY_FUNC (name) (lua_State *L)		\
  {								\
    return DECLINE (gener_raw_2p) (L, MY_GSL_FUNC (name));	\
  }

#define EXPAND(n) MY_DECLARE(n)
#define EXPAND_2P(n) MY_DECLARE(n)
#define EXPAND_OTHER(n) MY_DECLARE(n)
#include "distributions-list.c"
#undef EXPAND
#undef EXPAND_2P
#undef EXPAND_OTHER


static const struct luaL_Reg DECLINE (functions)[] = {
#define EXPAND(n) REG_MY_DECLARE(n)
#define EXPAND_2P(n) REG_MY_DECLARE(n)
#define EXPAND_OTHER(n) REG_MY_DECLARE(n)
#include "distributions-list.c"
#undef EXPAND
#undef EXPAND_2P
#undef EXPAND_OTHER
  {NULL, NULL}
};

static int
DECLINE (gener_raw) (lua_State *L, my_gsl_func_t func)
{
  double x = luaL_checknumber (L, 1);
  double param = luaL_optnumber (L, 2, 1.0);
  double v = func (x, param);
  lua_pushnumber (L, v);
  return 1;
}

static int
DECLINE (gener_raw_2p) (lua_State *L, my_gsl_func_2p_t func)
{
  double x  = luaL_checknumber (L, 1);
  double p1 = luaL_checknumber (L, 2);
  double p2 = luaL_checknumber (L, 3);
  double v = func (x, p1, p2);
  lua_pushnumber (L, v);
  return 1;
}

#define EXPAND(n) MY_FUNC_IMPLEMENT(n)
#define EXPAND_2P(n) MY_FUNC_IMPLEMENT_2P(n)
#define EXPAND_OTHER(n)
#include "distributions-list.c"
#undef EXPAND
#undef EXPAND_2P
#undef EXPAND_OTHER

int
MY_FUNC (binomial) (lua_State *L)
{
  int n = luaL_checkinteger (L, 1);
  double p1 = luaL_checknumber (L, 2);
  int p2 = luaL_checkinteger (L, 3);
  double v;

  if (p2 < 0)
    luaL_error (L, "parameter n cannot be negative for binomial distribution");

  v = MY_GSL_FUNC (binomial) (n, p1, (unsigned int) p2);
  lua_pushnumber (L, v);
  return 1;
}

static int
MY_FUNC (poisson) (lua_State *L)
{
  int n = luaL_checkinteger (L, 1);
  double param = luaL_optnumber (L, 2, 1.0);
  double v = MY_GSL_FUNC (poisson) (n, param);
  lua_pushnumber (L, v);
  return 1;
}

void
INCLINE (register) (lua_State *L)
{
  lua_newtable (L);
  luaL_register (L, NULL, DECLINE(functions));
  lua_setfield (L, -2, MODULE_NAME);
}
