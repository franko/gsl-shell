
typedef int (*gsh_d_fun_t)(double, gsl_sf_result *);
typedef int (*gsh_u_fun_t)(unsigned int, gsl_sf_result *);

static int
push_gsl_result (lua_State *L, gsl_sf_result *r)
{
  SF_RETURN(r);
}

static int
gsh_sf_d_raw (lua_State *L, const char *fname, gsh_d_fun_t f_gsl)
{
  double x = luaL_checknumber (L, 1);		
  gsl_sf_result res;				
  int status = f_gsl(x, &res);
  if (status != GSL_SUCCESS)				       
    luaL_error (L, "%s: %s", fname, gsl_strerror (status));   

  SF_RETURN(&res);
}

static int
gsh_sf_u_raw (lua_State *L, const char *fname, gsh_u_fun_t f_gsl)
{
  unsigned int i = my_checkuint (L, 1);		
  gsl_sf_result res;				
  int status = f_gsl(i, &res);
  if (status != GSL_SUCCESS)				       
    luaL_error (L, "%s: %s", fname, gsl_strerror (status));   
  SF_RETURN(&res);
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
    SF_RETURN(&res); \
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
    SF_RETURN(&res); \
  }

#define _GSH_SF_I(gsl_name, lua_name)			\
  int my_ ## lua_name(lua_State *L)			\
  {							\
    int i = luaL_checknumber (L, 1);			\
    gsl_sf_result res;					\
    int status = GGSL_SF_NAME(gsl_name) (i, &res);		      \
    if (status != GSL_SUCCESS)					      \
      luaL_error (L, #lua_name ": %s", gsl_strerror (status));			      \
    SF_RETURN(&res); \
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
    SF_RETURN(&res); \
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
    SF_RETURN(&res); \
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
    SF_RETURN(&res); \
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
    SF_RETURN(&res); \
  }

#define _GSH_SF_DDDD(gsl_name, lua_name)		\
  int my_ ## lua_name(lua_State *L)			\
  {						\
    double a = luaL_checknumber (L, 1);		\
    double b = luaL_checknumber (L, 2);		\
    double c = luaL_checknumber (L, 3);		\
    double d = luaL_checknumber (L, 4);		\
    gsl_sf_result res;				\
    int status = GGSL_SF_NAME(gsl_name) (a, b, c, d, &res);	      \
    if (status != GSL_SUCCESS)					      \
      luaL_error (L, #lua_name ": %s", gsl_strerror (status));	      \
    SF_RETURN(&res); \
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
    SF_RETURN(&res); \
  }

#define _GSH_SF_CUSTOM(lua_name)

int GSH_LUA_NAME(fermi_dirac) (lua_State *L)
{
  double x = luaL_checknumber (L, 2);
  gsl_sf_result res;				\
  int status;

  if (lua_isnumber (L, 1))
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
  SF_RETURN(&res);
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

  SF_RETURN(&res);
}

int GSH_LUA_NAME(hyperg1F1) (lua_State *L)
{
  double x = luaL_checknumber (L, 3);
  gsl_sf_result res;				\
  int status;

  if (lua_isnumber (L, 1) && lua_isnumber (L, 2))
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

  if (lua_isnumber (L, 1) && lua_isnumber (L, 2))
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

int GSH_LUA_NAME(hyperg2F1conj) (lua_State *L)
{
  Complex a = luaL_checkcomplex(L, 1);
  double c = luaL_checknumber(L, 2);
  double x = luaL_checknumber (L, 3);
  gsl_sf_result res;
  int status;

  status = gsl_sf_hyperg_2F1_conj_e (creal(a), cimag(a), c, x, &res);

  if (status != GSL_SUCCESS)
    return luaL_error (L, "hyperg2F1conj: %s", gsl_strerror (status));

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

#define BESSEL_KIND(letter)					\
  int GSH_LUA_NAME(bessel ## letter) (lua_State *L)		\
  {								\
    int n = luaL_checkinteger (L, 1);	\
    double x = luaL_checknumber (L, 2); \
    gsl_sf_result res;			\
    int status;				\
    switch (n)				\
      {					\
      case 0:						\
      status = gsl_sf_bessel_ ## letter ## 0_e (x, &res);	\
      break; \
      case 1:						\
      status = gsl_sf_bessel_ ## letter ## 1_e (x, &res);	\
      break; \
    default: \
      status = gsl_sf_bessel_ ## letter ## n_e (n, x, &res); \
    }; \
  if (status != GSL_SUCCESS) \
    return luaL_error (L, "bessel" #letter ": %s", gsl_strerror (status)); \
  return push_gsl_result (L, &res); \
}

BESSEL_KIND(J)
BESSEL_KIND(Y)
BESSEL_KIND(I)
BESSEL_KIND(K)

int GSH_LUA_NAME(besselJzero) (lua_State *L)
{
  double nu = luaL_checknumber (L, 1);
  int is = luaL_checkinteger (L, 2);
  unsigned int s;
  gsl_sf_result res;
  int status;

  if (is <= 0)
    return luaL_typerror (L, 1, "positive integer");

  s = (unsigned int) is;

  if (nu == 0.0)
    {
      status = gsl_sf_bessel_zero_J0_e (s, &res);
    }
  else if (nu == 1.0)
    {
      status = gsl_sf_bessel_zero_J1_e (s, &res);
    }
  else
    {
      status = gsl_sf_bessel_zero_Jnu_e (nu, s, &res);
    }

  if (status != GSL_SUCCESS)
    return luaL_error (L, "besselJzero: %s", gsl_strerror (status));

  return push_gsl_result (L, &res);
}

int GSH_LUA_NAME(zeta) (lua_State *L)
{
  gsl_sf_result res;
  int status;

  if (lua_isnumber (L, 1))
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

int GSH_LUA_NAME(expint) (lua_State *L)
{
  int n = luaL_checkinteger (L, 1);
  double x = luaL_checknumber (L, 2);
  gsl_sf_result res;
  int status;

  switch (n)
    {
    case 1:
      status = gsl_sf_expint_E1_e (x, &res);
      break;
    case 2:
      status = gsl_sf_expint_E2_e (x, &res);
      break;
    default:
      status = gsl_sf_expint_En_e (n, x, &res);
    };

  if (status != GSL_SUCCESS)
    return luaL_error (L, "expint: %s", gsl_strerror (status));

  return push_gsl_result (L, &res);
}

int GSH_LUA_NAME(legendreP) (lua_State *L)
{
  int n = luaL_checkinteger (L, 1);
  double x = luaL_checknumber (L, 2);
  gsl_sf_result res;
  int status;

  switch (n)
    {
    case 1:
      status = gsl_sf_legendre_P1_e (x, &res);
      break;
    case 2:
      status = gsl_sf_legendre_P2_e (x, &res);
      break;
    case 3:
      status = gsl_sf_legendre_P3_e (x, &res);
      break;
    default:
      status = gsl_sf_legendre_Pl_e (n, x, &res);
    };

  if (status != GSL_SUCCESS)
    return luaL_error (L, "legendreP: %s", gsl_strerror (status));

  return push_gsl_result (L, &res);
}

int GSH_LUA_NAME(legendreQ) (lua_State *L)
{
  int n = luaL_checkinteger (L, 1);
  double x = luaL_checknumber (L, 2);
  gsl_sf_result res;
  int status;

  switch (n)
    {
    case 0:
      status = gsl_sf_legendre_Q0_e (x, &res);
      break;
    case 1:
      status = gsl_sf_legendre_Q1_e (x, &res);
      break;
    default:
      status = gsl_sf_legendre_Ql_e (n, x, &res);
    };

  if (status != GSL_SUCCESS)
    return luaL_error (L, "legendreQ: %s", gsl_strerror (status));

  return push_gsl_result (L, &res);
}

int GSH_LUA_NAME(cdilog) (lua_State *L)
{
  Complex z = luaL_checkcomplex (L, 1);
  gsl_sf_result rr, ri;
  double zr = creal(z), zi = cimag(z);
  double r = sqrt(zr*zr+zi*zi), th = atan2(cimag(z), creal(z));
  int status;

  status = gsl_sf_complex_dilog_e (r, th, &rr, &ri);

  if (status != GSL_SUCCESS)
    return luaL_error (L, "cdilog: %s", gsl_strerror (status));

  lua_pushcomplex (L, rr.val + I * ri.val);
  return 1;
}
