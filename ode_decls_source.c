
#define ODE_DEFAULT_EPS_ABS 1e-4
#define ODE_DEFAULT_EPS_REL 0.0
#define ODE_DEFAULT_STEP 0.1
#define ODE_DEFAULT_METHOD "rk8pd"

static int FUNCTION (ode, evolve) (lua_State *L);
static int FUNCTION (ode, set)    (lua_State *L);
static int FUNCTION (ode, new)    (lua_State *L);
static int FUNCTION (ode, index)  (lua_State *L);
static int FUNCTION (ode, free)  (lua_State *L);

static int FUNCTION (ode, get_t)  (lua_State *L);
static int FUNCTION (ode, get_y)  (lua_State *L);

static struct solver * FUNCTION (ode, check) (lua_State *L, int index);

enum fenv_e {
  FENV_Y = 0,
  FENV_Y_BUFFER,
  FENV_DYDT,
  FENV_DFDY,
  FENV_DFDY_BUFFER,
  FENV_DFDT,
  FENV_F,
  FENV_DF,
};

static const struct luaL_Reg FUNCTION (ode, methods)[] = {
  {"set",           FUNCTION (ode, set)},
  {"evolve",        FUNCTION (ode, evolve)},
  {"__gc",          FUNCTION (ode, free)},
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

static struct solver_type TYPE (ode_solver_type)[1] = {{"GSL." PREFIX "ode"}};
