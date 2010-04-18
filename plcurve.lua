
-- plcurve.lua
--  
-- Copyright (C) 2009, 2010 Francesco Abbate
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
--

local M = {}

local function scalar(x, y) return prod(x, y)[1] end

local function square(x) return prod(x, x)[1] end

local function stepper(f, p0, step, z_spacing)
   local s = {f         = f, 
	      p         = p0, 
	      z0        = f(p0), 
	      step      = step, 
	      z_spacing = z_spacing,
	      z_tol     = z_spacing * 1e-6}
   return s
end

local function qeval_closure(f, f0, p, d)
   local pe = p:copy()
   local set = function(q)
		  pe:set(1,1, p[1] + q * d[1])
		  pe:set(2,1, p[2] + q * d[2])
		end
   local eval  = function(q) set(q); return f(pe) - f0 end
   local yield = function(q) set(q); return pe end
   return eval, yield
end

local function quad_root_solve(f, f0, p, d)
   local qeval, yield = qeval_closure(f, f0, p, d)
   local fl, fc, fr = qeval(-1), qeval(0), qeval(1)
   local a0, a1, a2 = (fr + 2*fc + fl)/4 - z0, (fr - fl)/2, (fr - 2*fc + fl)/4
   local q = -(a0-a2)/a1 - 2*a2*(a0-a2)^2/a1^3 - 8*a2^2*(a0-a2)^3/a1^5
   return yield(q)
end

-- ORIGINAL algorithm
local function segment_root_francesco(f, z0, p0, d0, z_eps)
   local p, d = p0:copy(), d0:copy()
   local qeval, yield = qeval_closure(f, z0, p, d)
   local fl, fc, fr

   -- exact solution for quadratic equation
   -- the problem is that is singular when a2 = 0
   -- local function quad_root_solve_raw()
   --    local a0, a1, a2 = (fr + 2*fc + fl)/4 - z0, (fr - fl)/2, (fr - 2*fc + fl)/4
   --    if abs(a2) > 0.05 * abs(a1) then return end
   --    local rad = sqrt(a1^2 - 8*a2*(a0 - a2))
   --    local q = a1 > 0 and (-a1 + rad)/(4*a2) or (-a1 - rad)/(4*a2)
   --    if q >= -1 and q <= 1 then return q end
   -- end

   local function quad_root_solve_raw()
      local a0, a1, a2 = (fr + 2*fc + fl)/4, (fr - fl)/2, (fr - 2*fc + fl)/4
      if abs(a2) > 0.05 * abs(a1) then return end
      local r0, r2 = (a0-a2)/a1, 2*a2/a1
      local q = -r0 - r2*r0^2 - 2*r2^2*r0^3
      if q >= -1 and q <= 1 then return q end
   end

   for k=1,10 do
      fl, fc, fr = qeval(-1), qeval(0), qeval(1)

      if     abs(fl) < z_eps then return yield(-1)
      elseif abs(fr) < z_eps then return yield( 1) end

      local q = quad_root_solve_raw()
      if q then return yield(q) end
--	 print('root:', -log(abs(f(pz) - z0)), f(pz), z0);	 

      d:set(1,1, 0.5*d[1])
      d:set(2,1, 0.5*d[2])
      
      if fc * fl < 0 then
	 p:set(1,1, p[1] - d[1])
	 p:set(2,1, p[2] - d[2])
      else
	 p:set(1,1, p[1] + d[1])
	 p:set(2,1, p[2] + d[2])
      end
   end

   error 'segment_solve failed to converge'
end

-- BRENT algorithm
local function segment_root_brent(f, z0, p, dv, z_eps)

   local function is_between(x, a, b)
      if b < a then a, b = b, a end
      return (x > a and x < b)
   end

   local qeval, yield = qeval_closure(f, z0, p, dv)

   local a, b = -1, 1
   local fa, fb = qeval(a), qeval(b)
   local del = 1e-3

   local function check_ab_exchange()
      if abs(fa) < abs(fb) then 
	 a, b = b, a
	 fa, fb = fb, fa 
      end
   end

   check_ab_exchange()

   local mflag = true
   local c, fc = a, fa
   local d
   while abs(fb) >= z_eps and abs(b-a) > del do
      local s
      if fa ~= fc and fb ~= fc then
	 s = a*fb*fc/((fa-fb)*(fa-fc)) + b*fa*fc/((fb-fa)*(fb-fc)) + c*fa*fb/((fc-fa)*(fc-fb))
      else
	 s = b - fb*(b-a)/(fb-fa)
      end

      if not is_between(s, (3*a+b)/4, b)
         or (    mflag and abs(s-b) >= abs(b-c)/2)
         or (not mflag and abs(s-b) >= abs(c-d)/2)
         or (    mflag and abs(b-c) < del)
         or (not mflag and abs(c-d) < del) then
	 s = (a+b)/2
	 mflag = true
      else
	 mflag = false
      end
      local fs = qeval(s)
      d = c
      c, fc = b, fb
      if fa * fs < 0 then b, fb = s, fs else a, fa = s, fs end
      check_ab_exchange()
   end

   return yield(b)
   --   print('root:', -log(abs(f(pz) - z0)), f(pz), z0)
end

M.segment_root = segment_root_francesco

function M.step(s, dir)
   local g, gt = new(2,1), new(2,1)

   s.f(s.p, g)

   local gnrm = g:norm()
   local u = dir * vector {g[2] / gnrm, - g[1] / gnrm}

   local zdelmax = s.z_spacing / 20
   local step = s.step
   local zr, pt
   for k=1,20 do
      pt = s.p + step * u
      zr = s.f(pt, gt) - s.z0
      if abs(zr) < zdelmax then
	 local abserr = 1e-4 * s.z_spacing / s.step
	 if abs(gt[1] - g[1]) < abserr + 0.05 * gnrm and 
	    abs(gt[1] - g[2]) < abserr + 0.05 * gnrm then
	    break
	 end
      end
      step = step / 2
   end

   pz = pt - (zr / square(gt)) * gt

-- the following steps does improve the estimation the search interval.
-- this is probably not needed.
-- s.f(pz, g)
-- g = (zr / square(g)) * g

   s.p = quad_root_solve(s.f, s.z0, pz, g)
end


function M.does_close(a, b, c, xy_spacing)
   local v, z = b - a, c - a
   local q = scalar(z, v) / square(v)
   if q > 0 and q <= 1 + 1e-4 then
      local w = vector {v[2], -v[1]}
      local p = scalar(z, w) / w:norm()
      if abs(p) < 0.1 * xy_spacing then return true end
   end
end

return M
