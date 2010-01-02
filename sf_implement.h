
typedef int (*gsh_d_fun_t)(double, gsl_sf_result *);
typedef int (*gsh_u_fun_t)(unsigned int, gsl_sf_result *);

static int
push_gsl_result (lua_State *L, gsl_sf_result *r)
{
  lua_pushnumber (L, r->val);
  lua_pushnumber (L, r->err);
  return 2;
}

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

#define _GSH_SF_CUSTOM(lua_name)

int GSH_LUA_NAME(fermi_dirac) (lua_State *L)
{
  double x = luaL_checknumber (L, 2);
  gsl_sf_result res;				\
  int status;

  if (lua_isinteger (L, 1))
    {
      int i = lua_tointeger (L, 1);
      switch (i)
	{
	case -1:
	  status = gsl_sf_fermi_dirac_m1_e (x, &res);
	  break;
	case 0:
	  status = gsl_sf_fermi_dirac_0_e (x, &res);
	  break;
	case 1:
	  status = gsl_sf_fermi_dirac_1_e (x, &res);
	  break;
	case 2:
	  status = gsl_sf_fermi_dirac_2_e (x, &res);
	  break;
	default:
	  status = gsl_sf_fermi_dirac_int_e (i, x, &res);
	};
    }
  else if (lua_isnumber (L, 1))
    {
      double ix = 2 * lua_tonumber (L, 1);
      if ((int) ix == ix)
	{
	  switch ((int) ix)
	    {
	    case -1:
	      status = gsl_sf_fermi_dirac_mhalf_e (x, &res);
	      break;
	    case 1:
	      status = gsl_sf_fermi_dirac_half_e (x, &res);
	      break;
	    case 3:
	      status = gsl_sf_fermi_dirac_3half_e (x, &res);
	      break;
	    default:
	      status = GSL_EDOM;
	    };
	}
      else
	{
	  status = GSL_EDOM;
	}
    }
  else
    {
      return luaL_typerror (L, 1, "integer or -1/2,1/2 or 3/2 half-integer number");
    }

  if (status != GSL_SUCCESS)
    luaL_error (L, "fermi_dirac: %s", gsl_strerror (status));
  lua_pushnumber (L, res.val);
  lua_pushnumber (L, res.err);
  return 2;
}

int GSH_LUA_NAME(debye) (lua_State *L)
{
  int i = luaL_checkinteger (L, 1);
  double x = luaL_checknumber (L, 2);
  gsl_sf_result res;				\
  int status;

  switch (i)
    {
    case 1:
      status = gsl_sf_debye_1_e (x, &res);
      break;
    case 2:
      status = gsl_sf_debye_2_e (x, &res);
      break;
    case 3:
      status = gsl_sf_debye_3_e (x, &res);
      break;
    case 4:
      status = gsl_sf_debye_3_e (x, &res);
      break;
    case 5:
      status = gsl_sf_debye_4_e (x, &res);
      break;
    case 6:
      status = gsl_sf_debye_5_e (x, &res);
      break;
    default:
      status = GSL_EDOM;
    };

  if (status != GSL_SUCCESS)
    luaL_error (L, "debye: %s", gsl_strerror (status));
  lua_pushnumber (L, res.val);
  lua_pushnumber (L, res.err);
  return 2;
}

int GSH_LUA_NAME(hyperg1F1) (lua_State *L)
{
  double x = luaL_checknumber (L, 3);
  gsl_sf_result res;				\
  int status;

  if (lua_isinteger (L, 1) && lua_isinteger (L, 2))
    {
      int m = lua_tointeger (L, 1);
      int n = lua_tointeger (L, 2);

      status = gsl_sf_hyperg_1F1_int_e (m, n, x, &res);
    }
  else
    {
      double m = luaL_checknumber (L, 1);
      double n = luaL_checknumber (L, 2);

      status = gsl_sf_hyperg_1F1_e (m, n, x, &res);
    }

  if (status != GSL_SUCCESS)
    return luaL_error (L, "hyperg1F1: %s", gsl_strerror (status));

  return push_gsl_result (L, &res);
}

int GSH_LUA_NAME(hypergU) (lua_State *L)
{
  double x = luaL_checknumber (L, 3);
  gsl_sf_result res;
  int status;

  if (lua_isinteger (L, 1) && lua_isinteger (L, 2))
    {
      int m = lua_tointeger (L, 1);
      int n = lua_tointeger (L, 2);

      status = gsl_sf_hyperg_U_int_e (m, n, x, &res);
    }
  else
    {
      double m = luaL_checknumber (L, 1);
      double n = luaL_checknumber (L, 2);

      status = gsl_sf_hyperg_U_e (m, n, x, &res);
    }

  if (status != GSL_SUCCESS)
    return luaL_error (L, "hypergU: %s", gsl_strerror (status));

  return push_gsl_result (L, &res);
}


int GSH_LUA_NAME(laguerre) (lua_State *L)
{
  int i = luaL_checkinteger (L, 1);
  double a = luaL_checknumber (L, 2);
  double x = luaL_checknumber (L, 3);
  gsl_sf_result res;				\
  int status;

  switch (i)
    {
    case 1:
      status = gsl_sf_laguerre_1_e (a, x, &res);
      break;
    case 2:
      status = gsl_sf_laguerre_2_e (a, x, &res);
      break;
    case 3:
      status = gsl_sf_laguerre_3_e (a, x, &res);
      break;
    default:
      status = gsl_sf_laguerre_n_e (i, a, x, &res);
    };

  if (status != GSL_SUCCESS)
    return luaL_error (L, "laguerre: %s", gsl_strerror (status));

  return push_gsl_result (L, &res);
}

int GSH_LUA_NAME(zeta) (lua_State *L)
{
  gsl_sf_result res;
  int status;

  if (lua_isinteger (L, 1))
    {
      int n = lua_tointeger (L, 1);
      status = gsl_sf_zeta_int_e (n, &res);
    }
  else
    {
      double s = luaL_checknumber (L, 1);
      status = gsl_sf_zeta_e (s, &res);
    }

  if (status != GSL_SUCCESS)
    return luaL_error (L, "zeta: %s", gsl_strerror (status));

  return push_gsl_result (L, &res);
}
