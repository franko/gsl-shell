 -- Copyright (C) 2009 Francesco Abbate
 -- 
 -- This program is free software; you can redistribute it and/or modify
 -- it under the terms of the GNU General Public License as published by
 -- the Free Software Foundation; either version 3 of the License, or (at
 -- your option) any later version.
 -- 
 -- This program is distributed in the hope that it will be useful, but
 -- WITHOUT ANY WARRANTY; without even the implied warranty of
 -- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 -- General Public License for more details.
 -- 
 -- You should have received a copy of the GNU General Public License
 -- along with this program; if not, write to the Free Software
 -- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

a = gsl.new_matrix(3, 2)
a:set(0, 0, 0.32)
a:set(0, 1, 4.44)
a:set(1, 0, -0.32)
a:set(1, 1, 1.14)
a:set(2, 0, 7.24)
a:set(2, 1, -0.79)

b = gsl.new_matrix(3, 2)
b:set(0, 0, 5.54)
b:set(0, 1, 2.21)
b:set(1, 0, 7.63)
b:set(1, 1, -1.12)
b:set(2, 0, 0)
b:set(2, 1, 4.47)

m = gsl.new_matrix(2, 2)
m:set(0, 0, 0.707)
m:set(0, 1, 0.707)
m:set(1, 0, -0.707)
m:set(1, 1, 0.707)
