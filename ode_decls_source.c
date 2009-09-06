
typedef struct {
  lua_State *L;
  TYPE (gsl_matrix) *y;
  TYPE (gsl_matrix) *ybuff;
#if MULTIPLICITY >= 2
  TYPE (gsl_matrix) *J;
#endif
  double t, h;
  size_t n; /* ODE system dimension */
} TYPE (ode_params);

#define ODE_DEFAULT_EPS_ABS 1e-4
#define ODE_DEFAULT_EPS_REL 0.0
#define ODE_DEFAULT_METHOD "rk8pd"

static int FUNCTION (ode, evolve) (lua_State *L);
static int FUNCTION (ode, set)    (lua_State *L);
static int FUNCTION (ode, new)    (lua_State *L);
static int FUNCTION (ode, index)  (lua_State *L);

static int FUNCTION (ode, get_t)  (lua_State *L);
static int FUNCTION (ode, get_y)  (lua_State *L);

static struct ode_solver * FUNCTION (ode, check) (lua_State *L, int index);


static TYPE (ode_params) * FUNCTION (ode_params, push)  (lua_State *L, 
							 size_t dim, double h);
static int                 FUNCTION (ode_params, free)  (lua_State *L);
static TYPE (ode_params) * FUNCTION (ode_params, check) (lua_State *L, int idx);

static const struct luaL_Reg FUNCTION (ode, methods)[] = {
  {"set",           FUNCTION (ode, set)},
  {"evolve",        FUNCTION (ode, evolve)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION (ode, properties)[] = {
  {"t",            FUNCTION (ode, get_t)},
  {"y",            FUNCTION (ode, get_y)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION (ode, functions)[] = {
  {PREFIX "ode",     FUNCTION (ode, new)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION (ode_params, methods)[] = {
  {"__gc",          FUNCTION (ode_params, free)},
  {NULL, NULL}
};

static char const * const TYPE (name_ode)        = "GSL." PREFIX "ode";
static char const * const TYPE (name_ode_params) = "GSL." PREFIX "odep";
