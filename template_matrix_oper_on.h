
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
#define SCALAR_OP add_constant
#define OP_NAME "addition"

#elif defined(OPER_MUL)
#define OPER mul_elements
#define SCALAR_OP scale
#define OP_NAME "multiplication"

#elif defined(OPER_SUB)
#define OPER sub
#undef SCALAR_OP
#define OP_NAME "subtraction"

#elif defined(OPER_DIV)
#define OPER div_elements
#undef SCALAR_OP
#define OP_NAME "division"

#else
#error matrix operation directive unknown
#endif
