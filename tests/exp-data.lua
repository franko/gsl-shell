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

require 'igsl'

exp_data = {
{0, 12.5614258},
{0.1, 10.5836024},
{0.2, 8.6503717},
{0.3, 7.1132118},
{0.4, 6.0118132},
{0.5, 4.7299734},
{0.6, 3.9992366},
{0.7, 3.5120203},
{0.8, 2.8505341},
{0.9, 2.3464507},
{1, 2.1775884},
{1.1, 1.6592287},
{1.2, 1.5781804},
{1.3, 1.1987201},
{1.4, 1.0110214},
{1.5, 0.7942879},
{1.6, 0.9221610},
{1.7, 0.5223632},
{1.8, 0.7384735},
{1.9, 0.3676248}
}

exp_data0 = {
{0, 12.3400000},
{0.1, 10.1842868},
{0.2, 8.4051618},
{0.3, 6.9368378},
{0.4, 5.7250199},
{0.5, 4.7248982},
{0.6, 3.8994909},
{0.7, 3.2182767},
{0.8, 2.6560658},
{0.9, 2.1920694},
{1, 1.8091299},
{1.1, 1.4930873},
{1.2, 1.2322552},
{1.3, 1.0169887},
{1.4, 0.8393278},
{1.5, 0.6927030},
{1.6, 0.5716925},
{1.7, 0.4718218},
{1.8, 0.3893977},
{1.9, 0.3213726},
}

dt = exp_data0

function expf(x, f, J)
   for k=1, 20 do
      local t = dt[k][1]
      local y = dt[k][2]
      local x0, x1 = x:get(0,0), x:get(1,0)
      local e = math.exp(x1 * t)
      if f then f:set(k-1, 0, x0 * e - y) end
      if J then
	 J:set(k-1, 0, e)
	 J:set(k-1, 1, x0 * t * e)
      end
   end
end

s = gsl.solver(20, 2)

xt = tvector {15.0, -2.2}
ft = gsl.matrix(20, 1)
jt = gsl.matrix(20, 2)

function sumsq(v)
   gsl.prod(v, v)
end
