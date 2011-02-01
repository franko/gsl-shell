/*
* lcomplex.c
* C99 complex nummbers for Lua
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 02 Nov 2009 23:15:43
* This code is hereby placed in the public domain.
*/

#include <math.h>

#include "lcomplex.h"

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

static int Ltostring(lua_State *L)		/** tostring(z) */
{
 Complex z=Z(1);
 double x=creal(z);
 double y=cimag(z);
 lua_settop(L,0);
 if (x!=0) lua_pushnumber(L,x);
 if (y!=0)
 {
  if (y==1)
  {
   if (x!=0) lua_pushliteral(L,"+");
  }
  else if (y==-1)
   lua_pushliteral(L,"-");
  else
  {
   if (y>0 && x!=0) lua_pushliteral(L,"+");
   lua_pushnumber(L,y);
  }
  lua_pushliteral(L,"i");
 }
 lua_concat(L,lua_gettop(L));
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
B(add)			/** __add(z,w) */
B(div)			/** __div(z,w) */
B(mul)			/** __mul(z,w) */
B(sub)			/** __sub(z,w) */
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
	{ "__add",	    Ladd	},
	{ "__div",	    Ldiv	},
	{ "__eq",	      Leq	},
	{ "__mul",	    Lmul	},
	{ "__sub",	    Lsub	},
	{ "__unm",	    Lneg	},
	{ "__pow",	    Lpow	},
	{ "__tostring",	Ltostring},
	{ NULL,		NULL	}
};


static const luaL_Reg lcomplex_functions[] =
{
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
