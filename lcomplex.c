/*
* lcomplex.c
* C99 complex nummbers for Lua
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 02 Nov 2009 23:15:43
* This code is hereby placed in the public domain.
*/

#include <math.h>

#include "lcomplex.h"
#include "matrix_arith.h"

#include "lua.h"
#include "lauxlib.h"

#include "gs-types.h"

#define Z(i)		Pget(L,i)
#define O(i)		luaL_optnumber(L,i,0)

#define cadd(z,w)	((z)+(w))
#define csub(z,w)	((z)-(w))
#define cmul(z,w)	((z)*(w))
#define cdiv(z,w)	((z)/(w))
#define cneg(z)		(-(z))
#define cconj		conj

static Complex Pget(lua_State *L, int i)
{
 switch (lua_type(L,i))
 {
  case LUA_TNUMBER:
  case LUA_TSTRING:
   return luaL_checknumber(L,i);
  default:
   return *((Complex*) gs_check_userdata (L, i, GS_COMPLEX));
 }
}

int lua_pushcomplex(lua_State *L, Complex z)
{
 Complex *p= gs_new_object (sizeof(Complex), L, GS_COMPLEX);
 *p=z;
 return 1;
}

static int Leq(lua_State *L)			/** __eq(z,w) */
{
 lua_pushboolean(L,Z(1)==Z(2));
 return 1;
}

static void
fmt_number (double x, char *buffer, size_t bufsize)
{
  size_t len = snprintf (buffer, bufsize, "%g", x);
  if (len+1 > bufsize)
    buffer[bufsize-1] = '\0';
}

#define FMTBUF_SIZE 32

static const char *
img_part (lua_State *L, double y, double eps, bool with_sign)
{
  double ay = (y >= 0.0 ? y : -y);
  const char *ysign = (y >= 0.0 ? (with_sign ? "+" : "") : "-");
 char buf[FMTBUF_SIZE];

  if (fabs(ay - 1.0) > eps)
    {
      fmt_number (ay, buf, FMTBUF_SIZE);
      lua_pushfstring (L, "%s%si", ysign, buf);
    }
  else
    {
      lua_pushfstring (L, "%si", ysign);
    }

  return lua_tostring (L, -1);
}

static int Ltostring(lua_State *L)		/** tostring(z) */
{
 Complex z = Z(1);
 double x = creal(z);
 double y = cimag(z);
 double eps;
 char buf[FMTBUF_SIZE];

 if (lua_isnoneornil (L, 2))
   {
     double nn = sqrt(x*x + y*y);
     eps = fmax(1.0e-8 * nn, 1.0e-16);
   }
 else
   {
     eps = luaL_checknumber (L, 2);
   }

 lua_settop (L, 0);

 if (fabs(y) > eps)
   {
     if (fabs(x) > eps)
       {
	 const char *img = img_part (L, y, eps, true);
	 fmt_number (x, buf, FMTBUF_SIZE);
	 lua_pushfstring (L, "%s%s", buf, img);
       }
     else
       {
	 img_part (L, y, eps, false);
       }
   }
 else
   {
     if (fabs(x) <= eps)
       {
	 lua_pushliteral (L, "0");
       }
     else
       {
	 fmt_number (x, buf, FMTBUF_SIZE);
	 lua_pushstring (L, buf);
       }
   }

 return 1;
}

int lua_iscomplex (lua_State *L, int i)
{
  if (lua_isnumber (L, i))
    return 1;
  else
  {
    void *p = gs_is_userdata (L, i, GS_COMPLEX);
    return (p != NULL);
  }
  return 0;
}

Complex luaL_checkcomplex (lua_State *L, int i)
{
	return Pget(L, i);
};

Complex lua_tocomplex (lua_State *L, int i)
{
  if (lua_isnumber (L, i))
    {
      double n = lua_tonumber (L, i);
      return n;
    }
  else if (gs_is_userdata (L, i, GS_COMPLEX))
  {
    Complex *p = lua_touserdata (L, i);
    return *p;
  }

  return 0;
}

#define A(f,e)	static int L##f(lua_State *L) { return lua_pushcomplex(L,e); }
#define B(f)	A(f,c##f(Z(1),Z(2)))
#define F(f)	A(f,c##f(Z(1)))
#define G(f)	static int L##f(lua_State *L) { lua_pushnumber(L,c##f(Z(1))); return 1; }

#define RFIMP(f,er,ez)  static int L##f(lua_State *L) { \
  if (lua_isnumber (L, 1)) \
    lua_pushnumber(L,er(lua_tonumber(L,1))); \
  else { \
    Complex *z = gs_check_userdata (L, 1, GS_COMPLEX); \
    lua_pushcomplex (L, ez(*z)); \
  } \
  return 1; \
}

#define RF(f)  RFIMP(f,f,c##f)

A(new,O(1)+O(2)*I)	/** new(x,y) */
F(neg)			/** __unm(z) */
G(abs)			/** abs(z) */
RF(acos)			/** acos(z) */
RF(acosh)		/** acosh(z) */
G(arg)			/** arg(z) */
RF(asin)			/** asin(z) */
RF(asinh)		/** asinh(z) */
RF(atan)			/** atan(z) */
RF(atanh)		/** atanh(z) */
F(conj)			/** conj(z) */
RF(cos)			/** cos(z) */
RF(cosh)			/** cosh(z) */
RF(exp)			/** exp(z) */
G(imag)			/** imag(z) */
RF(log)			/** log(z) */
B(pow)			/** pow(z,w) */
F(proj)			/** proj(z) */
G(real)			/** real(z) */
RF(sin)			/** sin(z) */
RF(sinh)			/** sinh(z) */
RF(sqrt)			/** sqrt(z) */
RF(tan)			/** tan(z) */
RF(tanh)			/** tanh(z) */

static const luaL_Reg lcomplex_methods[] =
{
	{ "__add",	    matrix_op_add	},
	{ "__div",	    matrix_op_div	},
	{ "__eq",	      Leq	},
	{ "__mul",	    matrix_op_mul	},
	{ "__sub",	    matrix_op_sub	},
	{ "__unm",	    Lneg	},
	{ "__pow",	    Lpow	},
	{ "__tostring",	Ltostring},
	{ NULL,		NULL	}
};


static const luaL_Reg lcomplex_functions[] =
{
	{ "tostring_eps",	Ltostring},
	{ "abs",	Labs	},
	{ "acos",	Lacos	},
	{ "acosh",	Lacosh	},
	{ "arg",	Larg	},
	{ "asin",	Lasin	},
	{ "asinh",	Lasinh	},
	{ "atan",	Latan	},
	{ "atanh",	Latanh	},
	{ "conj",	Lconj	},
	{ "cos",	Lcos	},
	{ "cosh",	Lcosh	},
	{ "exp",	Lexp	},
	{ "imag",	Limag	},
	{ "log",	Llog	},
	{ "complex",	Lnew	},
	{ "pow",	Lpow	},
	{ "proj",	Lproj	},
	{ "real",	Lreal	},
	{ "sin",	Lsin	},
	{ "sinh",	Lsinh	},
	{ "sqrt",	Lsqrt	},
	{ "tan",	Ltan	},
	{ "tanh",	Ltanh	},
	{ NULL,		NULL	}
};

void lcomplex_register (lua_State *L)
{
  luaL_newmetatable (L, GS_METATABLE(GS_COMPLEX));
  luaL_register (L, NULL, lcomplex_methods);
  lua_pop (L, 1);

  luaL_register(L, NULL, lcomplex_functions);

  lua_pushcomplex(L, I);
  lua_setfield(L, -2, "I");
}
