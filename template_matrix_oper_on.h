
/* template_matrix_oper_on.h
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

#if defined(OPER_ADD)
#define OPER add
#define OPER_ELEM add
#define OP_ELEM_DEF 1
#define SCALAR_OP add_constant
#define OP_NAME "addition"
#define BASE_OPER(a,b) ((a) + (b))

#elif defined(OPER_MUL)
#define OPER mul
#define OPER_ELEM mul_elements
#define OP_ELEM_DEF 0
#define SCALAR_OP scale
#define OP_NAME "multiplication"
#define BASE_OPER(a,b) ((a) * (b))

#elif defined(OPER_SUB)
#define OPER sub
#define OPER_ELEM sub
#define OP_ELEM_DEF 1
#define SCALAR_OP add_constant
#define OP_NAME "subtraction"
#define BASE_OPER(a,b) ((a) - (b))

#elif defined(OPER_DIV)
#define OPER div
#define OPER_ELEM div_elements
#define OP_ELEM_DEF 0
#define SCALAR_OP scale
#define OP_NAME "division"
#define BASE_OPER(a,b) ((a) / (b))

#else
#error matrix operation directive unknown
#endif

#define CONCAT2x(a,b) a ## _ ## b 
#define CONCAT2(a,b) CONCAT2x(a,b)

#define OPER_FUNCTION(base) CONCAT2(base,OPER_ELEM)
#define SCALAR_MAT_FUNCTION(base) CONCAT2(base,OPER)
