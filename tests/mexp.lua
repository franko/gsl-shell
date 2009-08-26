
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

function mexp(x, nmax)
  local n = x:dims()
  local r = unit(n)
  local p = r
  local f = 1
  for i=1, nmax do
    p = gsl.mul(p, x)
    f = f / i
    r = r + f * p
  end
  return r
end

th = gsl.matrix (2, 2)
th:set(0,0,  0.0)
th:set(0,1,  0.3)
th:set(1,0, -0.3)
th:set(1,1,  0.0)

cr = tmatrix {{1.0000, -0.0985, 0.0600, -0.0412}, {-0.0985, 1.0000, -0.9865, -0.0923}, {0.0600, -0.9865, 1.0000, 0.1273}, {-0.0412, -0.0923, 0.1273, 1}}

sg = tvector {0.0127919, 0.0682126, 0.562766, 0.000654265}
