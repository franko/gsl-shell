
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

function mexpff(m, imax)
   local n = m:dims()
   local function fexp(accu, p, f, i)
      if i >= imax then return accu else
	 p, f = mul(p, m), f/i
	 return fexp(accu + f * p, p, f, i+1)
      end
   end
   local u = unit(n)
   return fexp(u, u, 1, 1)
end

function mexp(m, nmax)
  local n = m:dims()
  local r = unit(n)
  local p, f = r, 1
  for i=1, nmax do
    p = mul(p, m)
    f = f / i
    r = r + f * p
  end
  return r
end

th = new(2, 2)
th:set(0,0,  0.0)
th:set(0,1,  0.3)
th:set(1,0, -0.3)
th:set(1,1,  0.0)

cr = matrix {{1.0000, -0.0985, 0.0600, -0.0412}, {-0.0985, 1.0000, -0.9865, -0.0923}, {0.0600, -0.9865, 1.0000, 0.1273}, {-0.0412, -0.0923, 0.1273, 1}}

sg = vector {0.0127919, 0.0682126, 0.562766, 0.000654265}
