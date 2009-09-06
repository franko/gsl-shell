
/* template_matrix_on.h
 * 
 * Copyright (C) 2009 Francesco Abbate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#define lua_iscomplex lua_isnumber

#if   defined(BASE_GSL_COMPLEX)
#undef complex
#define BASE gsl_complex
#define SHORT complex
#define MULTIPLICITY 2
#define ONE {{1.0, 0.0}}
#define LUA_SHORT complex
#define LUA_SHORTM Complex
#define BLAS_ID z
#define PREFIX "c"
#define BASE_TYPE TYPE_COMPLEX

#elif defined(BASE_DOUBLE)
#define BASE double
#define SHORT
#define MULTIPLICITY 1
#define ONE 1.0
#define LUA_SHORT number
#define LUA_SHORTM Number
#define BLAS_ID d
#define PREFIX ""
#define BASE_TYPE TYPE_REAL

#define value_retrieve(x) (x)
#define value_assign(x) (x)

#else
#error unknown BASE_ directive
#endif

#define CONCAT2x(a,b) a ## _ ## b 
#define CONCAT2(a,b) CONCAT2x(a,b)
#define CONCAT3x(a,b,c) a ## _ ## b ## _ ## c
#define CONCAT3(a,b,c) CONCAT3x(a,b,c)
#define CONCAT4x(a,b,c,d) a ## _ ## b ## _ ## c ## _ ## d
#define CONCAT4(a,b,c,d) CONCAT4x(a,b,c,d)
#define MYCATx(a,b) a ## b
#define MYCAT(a,b) MYCATx(a,b)

#if defined(BASE_DOUBLE)
#define FUNCTION(dir,name) CONCAT2(dir,name)
#define CONST_FUNCTION(dir,name) CONCAT3(dir,const,name)
#define TYPE(dir) dir
#define VIEW(dir) CONCAT2(dir,view)
#define CONST_VIEW(dir) CONCAT3(dir,const,view)
#else
#define FUNCTION(a,c) CONCAT3(a,SHORT,c)
#define CONST_FUNCTION(a,c) CONCAT4(a,SHORT,const,c)
#define TYPE(dir) CONCAT2(dir,SHORT)
#define VIEW(dir) CONCAT3(dir,SHORT,view)
#define CONST_VIEW(dir) CONCAT4(dir,SHORT,const,view)
#endif

#define LUA_TYPE CONCAT2(lua,LUA_SHORTM)
#define LUA_FUNCTION(oper) CONCAT2(lua,MYCAT(oper,LUA_SHORT))
#define LUAL_FUNCTION(oper) CONCAT2(luaL,MYCAT(oper,LUA_SHORT))
#define BLAS_FUNCTION(name) CONCAT2(gsl_blas,MYCAT(BLAS_ID,name))

