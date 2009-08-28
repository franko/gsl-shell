
static char const * const TYPE (name_solver) = "GSL." PREFIX "solver";

/* declaration of lua function for solver methods */
static int FUNCTION (solver, new)                (lua_State *);
static int FUNCTION (solver, set)                (lua_State *);
static int FUNCTION (solver, run)                (lua_State *);
static int FUNCTION (solver, iterate)            (lua_State *);
static int FUNCTION (solver, index)              (lua_State *);
static int FUNCTION (solver, covar)              (lua_State *);
static int FUNCTION (solver, get_x)              (lua_State *);
static int FUNCTION (solver, get_f)              (lua_State *);
static int FUNCTION (solver, get_jacob)          (lua_State *);

static const struct luaL_Reg FUNCTION (solver, methods)[] = {
  {"set",          FUNCTION (solver, set)},
  {"iterate",      FUNCTION (solver, iterate)},
  {"run",          FUNCTION (solver, run)},
  {NULL, NULL}
};  

static const struct luaL_Reg FUNCTION (solver, properties)[] = {
  {"covar",        FUNCTION (solver, covar)},
  {"x",            FUNCTION (solver, get_x)},
  {"f",            FUNCTION (solver, get_f)},
  {"J",            FUNCTION (solver, get_jacob)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION (solver, functions)[] = {
  {PREFIX "solver",      FUNCTION (solver, new)},
  {NULL, NULL}
};
