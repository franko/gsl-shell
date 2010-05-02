
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

local function set_lininterp(p, p1, p2, a)
   p:set(1,1, (1-a) * p1[1] + a * p2[1])
   p:set(2,1, (1-a) * p1[2] + a * p2[2])
end

local function set_affine_trans(p, p1, d, q)
   p:set(1,1, p1[1] + q * d[1])
   p:set(2,1, p1[2] + q * d[2])
end

local function translate(p, d, q)
   p:set(1,1, p[1] + q * d[1])
   p:set(2,1, p[2] + q * d[2])
end

local function scale(v, a)
   v:set(1,1, a * v[1])
   v:set(2,1, a * v[2])
end

local function set_orthogonal(v, w, a)
   v:set(1,1,   a * w[2])
   v:set(2,1, - a * w[1])
end

local function qeval_closure(f, f0, p, d)
   local pe = p:copy()
   local set   = function(q) set_affine_trans(pe, p, d, q) end
   local eval  = function(q) set(q); return f(pe) - f0 end
--   local diag  = function(q) print('>>', -log(abs(f(pe) - f0)), f(pe), f0) end
   local yield = function(q) set(q); return pe end
   return eval, yield
end

local function quad_root_solve(f, f0, p, d)
   local qeval, yield = qeval_closure(f, f0, p, d)
   local fl, fc, fr = qeval(-1), qeval(0), qeval(1)
   local a0, a1, a2 = (fr + 2*fc + fl)/4, (fr - fl)/2, (fr - 2*fc + fl)/4
   local r0, r2 = (a0-a2)/a1, 2*a2/a1
   local q = -r0 - r2*r0^2 - 2*r2^2*r0^3
   return yield(q)
end

-- ORIGINAL algorithm
local function segment_root_francesco(f, z0, p0, d0, z_eps)
   local p, d = p0:copy(), d0:copy()
   local qeval, yield = qeval_closure(f, z0, p, d)
   local fl, fc, fr

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
      if q then 
	 return yield(q)
      else
	 scale(d, 0.5)
	 local sign = (fc * fl < 0 and -1 or 1)
	 translate(p, d, sign)
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
end

M.segment_root = segment_root_brent

function M.stepper(f, p0, step0, z_spacing)

   local p, z0 = p0:copy(), f(p0)
   local z_tol = z_spacing * 1e-6
   local zdelmax = z_spacing / 20
   local abserr = 1e-4 * z_spacing / step0

   local g, gt, u = new(2,1), new(2,1), new(2,1)
   local pt = new(2,1)

   local function stepper_advance(dir)

      f(p, g)

      local gnrm = g:norm()
      set_orthogonal(u, g, dir / gnrm)

      local zr
      local step = step0

      for k=1,20 do
	 set_affine_trans(pt, p, u, step)
	 zr = f(pt, gt) - z0
	 if abs(zr) < zdelmax then
	    if abs(gt[1] - g[1]) < abserr + 0.05 * gnrm and 
	       abs(gt[2] - g[2]) < abserr + 0.05 * gnrm then
	    break
	    end
	 end
	 step = step / 2
      end

      scale(gt, zr/square(gt))
      translate(pt, gt, -1)

      -- the following steps does improve the estimation the search interval.
      -- this is probably not needed.
      -- f(pt, gt)
      -- scale(gt, zr/square(gt))

      set(p, quad_root_solve(f, z0, pt, gt))
   end

   local function stepper_point()
      return p
   end

   local function stepper_set(px)
      set(p, px)
   end

   return {advance= stepper_advance,
	   point  = stepper_point,
	   set    = stepper_set}
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
