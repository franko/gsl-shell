
typedef int (*gsh_d_fun_t)(double, gsl_sf_result *);
typedef int (*gsh_u_fun_t)(unsigned int, gsl_sf_result *);

static int
gsh_sf_d_raw (lua_State *L, const char *fname, gsh_d_fun_t f_gsl)
{
  double x = luaL_checknumber (L, 1);		
  gsl_sf_result res;				
  int status = f_gsl(x, &res);
  if (status != GSL_SUCCESS)				       
    luaL_error (L, "%s: %s", fname, gsl_strerror (status));   
  lua_pushnumber (L, res.val);	        
  lua_pushnumber (L, res.err);		
  return 2;					
}

static int
gsh_sf_u_raw (lua_State *L, const char *fname, gsh_u_fun_t f_gsl)
{
  unsigned int i = my_checkuint (L, 1);		
  gsl_sf_result res;				
  int status = f_gsl(i, &res);
  if (status != GSL_SUCCESS)				       
    luaL_error (L, "%s: %s", fname, gsl_strerror (status));   
  lua_pushnumber (L, res.val);	        
  lua_pushnumber (L, res.err);		
  return 2;					
}

#define _GSH_SF_D(gsl_name, lua_name)			\
  int my_ ## lua_name(lua_State *L)			\
  {						\
    return gsh_sf_d_raw(L, #lua_name, GGSL_SF_NAME(gsl_name));	\
  }

#define _GSH_SF_U(gsl_name, lua_name)			\
  int my_ ## lua_name(lua_State *L)			\
  {							\
    return gsh_sf_u_raw(L, #lua_name, GGSL_SF_NAME(gsl_name));	\
  }

#define _GSH_SF_UU(gsl_name, lua_name)			\
  int my_ ## lua_name(lua_State *L)			\
  {							\
    unsigned int a = my_checkuint (L, 1);		\
    unsigned int b = my_checkuint (L, 2);		\
    gsl_sf_result res;					\
    int status = GGSL_SF_NAME(gsl_name) (a, b, &res);		      \
    if (status != GSL_SUCCESS)					      \
      luaL_error (L, #lua_name ": %s", gsl_strerror (status)); \
    lua_pushnumber (L, res.val);				\
    lua_pushnumber (L, res.err);				\
    return 2;							\
  }


#define _GSH_SF_DU(gsl_name, lua_name)			\
  int my_ ## lua_name(lua_State *L)			\
  {							\
    double x = luaL_checknumber (L, 1);			\
    unsigned int i = my_checkuint (L, 2);		\
    gsl_sf_result res;					\
    int status = GGSL_SF_NAME(gsl_name) (x, i, &res);		      \
    if (status != GSL_SUCCESS)						\
      luaL_error (L, #lua_name ": %s", gsl_strerror (status));		\
    lua_pushnumber (L, res.val);				\
    lua_pushnumber (L, res.err);				\
    return 2;							\
  }

#define _GSH_SF_I(gsl_name, lua_name)			\
  int my_ ## lua_name(lua_State *L)			\
  {							\
    int i = luaL_checknumber (L, 1);			\
    gsl_sf_result res;					\
    int status = GGSL_SF_NAME(gsl_name) (i, &res);		      \
    if (status != GSL_SUCCESS)					      \
      luaL_error (L, #lua_name ": %s", gsl_strerror (status));			      \
    lua_pushnumber (L, res.val);				\
    lua_pushnumber (L, res.err);				\
    return 2;							\
  }

#define _GSH_SF_ID(gsl_name, lua_name)			\
  int my_ ## lua_name(lua_State *L)			\
  {							\
    int i = luaL_checknumber (L, 1);		\
    double x = luaL_checknumber (L, 2);		\
    gsl_sf_result res;				\
    int status = GGSL_SF_NAME(gsl_name) (i, x, &res);	\
    if (status != GSL_SUCCESS)					      \
      luaL_error (L, #lua_name ": %s", gsl_strerror (status)); \
    lua_pushnumber (L, res.val);	        \
    lua_pushnumber (L, res.err);		\
    return 2;					\
  }

#define _GSH_SF_D_MODE(gsl_name, lua_name)		\
  int my_ ## lua_name(lua_State *L)				\
  {							\
    double x = luaL_checknumber (L, 1);			\
    const char *s = luaL_optstring (L, 2, NULL);	\
    gsl_mode_t mode = gsl_mode_from_string (s);		\
    gsl_sf_result res;					\
    int status = GGSL_SF_NAME(gsl_name) (x, mode, &res);	      \
    if (status != GSL_SUCCESS)					      \
      luaL_error (L, #lua_name ": %s", gsl_strerror (status));			      \
    lua_pushnumber (L, res.val);			\
    lua_pushnumber (L, res.err);			\
    return 2;						\
  }


#define _GSH_SF_DD(gsl_name, lua_name)		\
  int my_ ## lua_name(lua_State *L)			\
  {						\
    double a = luaL_checknumber (L, 1);		\
    double b = luaL_checknumber (L, 2);		\
    gsl_sf_result res;				\
    int status = GGSL_SF_NAME(gsl_name) (a, b, &res);	\
    if (status != GSL_SUCCESS)					      \
      luaL_error (L, #lua_name ": %s", gsl_strerror (status)); \
    lua_pushnumber (L, res.val);	        \
    lua_pushnumber (L, res.err);		\
    return 2;					\
  }

#define _GSH_SF_DDD(gsl_name, lua_name)		\
  int my_ ## lua_name(lua_State *L)			\
  {						\
    double a = luaL_checknumber (L, 1);		\
    double b = luaL_checknumber (L, 2);		\
    double c = luaL_checknumber (L, 3);		\
    gsl_sf_result res;				\
    int status = GGSL_SF_NAME(gsl_name) (a, b, c, &res);	      \
    if (status != GSL_SUCCESS)					      \
      luaL_error (L, #lua_name ": %s", gsl_strerror (status));			      \
    lua_pushnumber (L, res.val);	        \
    lua_pushnumber (L, res.err);		\
    return 2;					\
  }


#define _GSH_SF_IID(gsl_name, lua_name)			\
  int my_ ## lua_name(lua_State *L)				\
  {							\
    int a = luaL_checknumber (L, 1);			\
    int b = luaL_checknumber (L, 2);			\
    double c = luaL_checknumber (L, 3);			\
    gsl_sf_result res;					\
    int status = GGSL_SF_NAME(gsl_name) (a, b, c, &res);		\
    if (status != GSL_SUCCESS)						\
      luaL_error (L, #lua_name ": %s", gsl_strerror (status));		\
    lua_pushnumber (L, res.val);			\
    lua_pushnumber (L, res.err);			\
    return 2;						\
  }
