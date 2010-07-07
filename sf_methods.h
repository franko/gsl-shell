
#define METHOD_LINE(lua_name) {#lua_name, GSH_LUA_NAME(lua_name)},

#define _GSH_SF_D(gsl_name, lua_name)      METHOD_LINE(lua_name)
#define _GSH_SF_U(gsl_name, lua_name)      METHOD_LINE(lua_name)
#define _GSH_SF_UU(gsl_name, lua_name)     METHOD_LINE(lua_name)
#define _GSH_SF_I(gsl_name, lua_name)      METHOD_LINE(lua_name)
#define _GSH_SF_DU(gsl_name, lua_name)     METHOD_LINE(lua_name)
#define _GSH_SF_D_MODE(gsl_name, lua_name) METHOD_LINE(lua_name)
#define _GSH_SF_ID(gsl_name, lua_name)     METHOD_LINE(lua_name)
#define _GSH_SF_DD(gsl_name, lua_name)     METHOD_LINE(lua_name)
#define _GSH_SF_DDD(gsl_name, lua_name)    METHOD_LINE(lua_name)
#define _GSH_SF_DDDD(gsl_name, lua_name)   METHOD_LINE(lua_name)
#define _GSH_SF_IID(gsl_name, lua_name)    METHOD_LINE(lua_name)
#define _GSH_SF_CUSTOM(lua_name)           METHOD_LINE(lua_name)
