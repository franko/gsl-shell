
-- contour.lua
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

require 'geom'

local point  = geom.point
local vector = geom.vector
local scalar = geom.scalarprod

-- local function quad_root_solve(f, f0, p, d)
--    local pl, pr = p-d, p+d
--    local fl, fc, fr = f(pl), f(p), f(pr)
--    local a0, a1, a2 = (fr + 2*fc + fl)/4 - f0, (fr - fl)/2, (fr - 2*fc + fl)/4
--    local q = -(a0 - a2)/a1 - 2*a2* (a0 - a2)^2 / a1^3
--    return p + q * d
-- end

local function quad_root_solve(f, f0, p, d)
   local pl, pr = p-d, p+d
   local fl, fc, fr = f(pl), f(p), f(pr)
   local a0, a1, a2 = (fr + 2*fc + fl)/4 - f0, (fr - fl)/2, (fr - 2*fc + fl)/4
   local rad = sqrt(a1^2 - 8*a2*(a0 - a2))
   local q = a1 > 0 and (-a1 + rad)/(4*a2) or (-a1 - rad)/(4*a2)
   return p + q * d
end

local function segment_solve(f, z0, p, d, z_eps)
   local pl, pr = p-d, p+d
   local fl, fc, fr

   local function quad_root_solve_raw(p, d)
      local a0, a1, a2 = (fr + 2*fc + fl)/4 - z0, (fr - fl)/2, (fr - 2*fc + fl)/4
      if abs(a2) > 0.01 * abs(a1) then return end
      local q = -(a0 - a2)/a1 - 2*a2* (a0 - a2)^2 / a1^3
      if q < -1 - 1e-4 or q > 1 + 1e-4 then return end
      return p + q * d
   end

   for k=1,10 do
      fl, fc, fr = f(pl), f(p), f(pr)

      if abs(fl - z0) < z_eps then 
	 return pl 
      elseif abs(fr - z0) < z_eps then
	 return pr
      end

      local pz = quad_root_solve_raw(p, d)
      if pz then return pz end
      
      if (fc - z0) * (fl - z0) < 0 then
	 p, pr = pl + 0.5 * d, p
      else
	 pl, p = p, pr - 0.5 * d
      end

      d = 0.5 * d
   end

   error 'segment_solve failed to converge'
end

-- local function segment_solve(f, f0, p, d)
--    local pz = quad_root_solve(f, f0, p, d)
--    local g = vector()
--    local z = f(pz, g) - f0
--    g = (z / g:square()) * g
--    d = (g:square() / scalar(g, d)) * d
--    pz = quad_root_solve(f, f0, pz - d, d)
--    return pz
-- end

local function contour_step(s, dir)
   local g, gt = vector(), vector()

   s.f(s.p, g)

   local gnrm = g:norm()
   local u = dir * vector(g.dy / gnrm, - g.dx / gnrm)

   local zdelmax = s.z_spacing / 20
   local step = s.step
   local zr, pt
   for k=1,20 do
      pt = s.p + step * u
      zr = s.f(pt, gt) - s.z0
      if abs(zr) < zdelmax then
	 local abserr = 1e-4 * s.z_spacing / s.step
	 if abs(gt.dx - g.dx) < abserr + 0.05 * gnrm and 
	    abs(gt.dy - g.dy) < abserr + 0.05 * gnrm then
	    break
	 end
      end

      step = step / 2
   end

   pz = pt - (zr / gt:square()) * gt
   s.f(pz, g)
   g = (zr / g:square()) * g

   s.p = quad_root_solve(s.f, s.z0, pz, g)
end

local function stepper(f, p0, step, z_spacing)
   local s = {f         = f, 
	      p         = p0, 
	      z0        = f(p0), 
	      step      = step, 
	      z_spacing = z_spacing,
	      z_tol     = z_spacing * 1e-6}
   return s
end

local function grid_create(f, left, right, nx, ny, nlevels)
   local g = {z= {}, cross= {}, zmin= f(left), zmax= f(right), levels= {}}
   local dx = vector((right.x - left.x) / nx, 0)
   local dy = vector(0, (right.y - left.y) / ny)
   local ds = sqrt(dx.dx^2 + dy.dy^2)

   local function index(i,j) return j + i * (nx+1) end
   local function segment_index(i,j,dir)
      local i = index(i,j)
      if dir == 'v' then i = i + (nx+1) * (ny+1) end
      return i
   end

   local function add_cross(g, i, j, dir, k)
      local si = segment_index(i, j, dir)
      if not g.cross[si] then g.cross[si] = {} end
      g.cross[si][k] = 1
   end

   local function edge_values(i, j, dir)
      if (dir == 'h' and (j >= nx or j < 0)) or (dir == 'v' and (i >= ny or i < 0)) then return end
      local i, inext = index(i,j), (dir == 'h' and index(i,j+1) or index(i+1,j))
      local a, b = g.z[i], g.z[inext]
      if a and b then
	 if a > b then a, b = b, a end
	 return a, b
      end
   end

   local function populate_grid(g, k, z)
      for i=0,ny do
	 for j=0,nx do
	    local a, b = edge_values(i, j, 'h')
	    if a and z >= a and z < b then
	       add_cross(g, i, j, 'h', k)
	    end
	    local a, b = edge_values(i, j, 'v')
	    if a and z >= a and z < b then
	       add_cross(g, i, j, 'v', k)
	    end
	 end
      end
   end

   for i= 0, ny do
      for j= 0, nx do
	 local p = left + j*dx + i*dy
	 local z = f(p)
	 if z < g.zmin then g.zmin = z end
	 if z > g.zmax then g.zmax = z end
	 g.z[index(i,j)] = z
      end
   end

   local zstep = (g.zmax - g.zmin) / nlevels
   local zeps = 1e-4 * zstep
   for k=0, nlevels do g.levels[k] = g.zmin + k * zstep end

   for k, z in pairs(g.levels) do populate_grid(g, k, z) end

   local function alpha(x, a, b, s) 
      local r = (x-a)/(b-a) 
      if r > 0 and r <= 1 and (x-a)*s > 0 then return r end
   end

   local function test(a, a1, a2)
      if a then return (not a1 or a < a1) and (not a2 or a < a2) end
   end

   local function grid_get_point_index(p)
      local jx, ix = (p.x - left.x) / dx.dx, (p.y - left.y) / dy.dy
      local c = {}
      c.i, c.j = floor(ix), floor(jx)
      c.di, c.dj = (ix - c.i > 0 and 1 or 0), (jx - c.j > 0 and 1 or 0)
      return c
   end

   local function segment_remove(i, j, dir, level)
      local si = segment_index(i, j, dir)
      if si and g.cross[si] then g.cross[si][level] = nil end
   end

   local function grid_remove_cross(p, level)
      local jx, ix = (p.x - left.x) / dx.dx, (p.y - left.y) / dy.dy
      local i, j = floor(ix+0.5), floor(jx+0.5)

      if abs(jx - j) < 0.05 then 
	 segment_remove(i, j,   'v', level) 
	 segment_remove(i-1, j, 'v', level) 
      end
      if abs(ix - i) < 0.05 then 
	 segment_remove(i, j,   'h', level)
	 segment_remove(i, j-1, 'h', level)
      end
   end      

   local function cut_at_boundary(p1, p2, alpha)
      local p = p1 + alpha * (p2 - p1)
      p2.x = p.x
      p2.y = p.y
      return true
   end

   local function grid_cross_check(cell, p1, p2)
      local a = left + cell.j * dx + cell.i * dy
      local b = a + dx + dy

      local axr = alpha(b.x, p1.x, p2.x,  1)
      local ayt = alpha(b.y, p1.y, p2.y,  1)
      local axl = alpha(a.x, p1.x, p2.x, -1)
      local ayb = alpha(a.y, p1.y, p2.y, -1)

      if test(axr, ayt, ayb) then
	 grid_remove_cross(p1 + axr * (p2 - p1), cell.level)
	 cell.j = cell.j + 1
	 if (cell.j >= nx) then return cut_at_boundary(p1, p2, axr) end
	 return grid_cross_check(cell, p1, p2)
      elseif test(ayt, axl, axr) then
	 grid_remove_cross(p1 + ayt * (p2 - p1), cell.level)
	 cell.i = cell.i + 1
	 if (cell.i >= ny) then return cut_at_boundary(p1, p2, ayt) end
	 return grid_cross_check(cell, p1, p2)
      elseif test(axl, ayt, ayb) then
	 grid_remove_cross(p1 + axl * (p2 - p1), cell.level)
	 cell.j = cell.j - 1
	 if (cell.j < 0) then return cut_at_boundary(p1, p2, axl) end
	 return grid_cross_check(cell, p1, p2)
      elseif test(ayb, axl, axr) then
	 grid_remove_cross(p1 + ayb * (p2 - p1), cell.level)
	 cell.i = cell.i - 1
	 if (cell.i < 0) then return cut_at_boundary(p1, p2, ayb) end
	 return grid_cross_check(cell, p1, p2)
      end

      return false
   end

   local function grid_next_point (level)
      for i= 0, ny do
	 for j= 0, nx do
	    local si = segment_index(i, j, 'h')
	    if g.cross[si] and g.cross[si][level] then
	       local p = left + (j+0.5)*dx + i*dy
	       p = segment_solve(f, g.levels[level], p, 0.5 * dx, zeps)
	       return p
	    end
	    si = segment_index(i, j, 'v')
	    if g.cross[si] and g.cross[si][level] then
	       local p = left + j*dx + (i+0.5)*dy
	       p = segment_solve(f, g.levels[level], p, 0.5 * dy, zeps)
	       return p
	    end
	 end
      end
   end

   local function start_point_iter()
      local level = 1
      local p
      return function()
	 while true do
	    p = grid_next_point(level)
	    if p then break end
	    level = level + 1
	    if level >= nlevels then return end
	 end
	 return p, level
      end
   end      

   local function grid_cell_verify(c)
      if c.j >= 0 and c.j + 1 <= nx then
   	 if c.i >= 0 and c.i + 1 <= ny then
   	    return true
   	 end
      end
   end

   local function grid_does_close(a, b, c)
      local v, z = b - a, c - a
      local q = scalar(z, v) / v:square()
      if q > 0 and q <= 1 then
	 local w = vector(v.dy, -v.dx)
	 local p = scalar(z, w) / w:square()
	 if abs(p) < 0.2 * ds then return true end
      end
   end

   local function debug_print_cross()
      local pl = plot()
      for k = 0, nlevels do
	 local line = path()
	 for i=0,ny do
	    for j=0,nx do
	       local ls = g.cross[segment_index(i,j,'h')]
	       if ls and ls[k] then
		  local p = left + j*dx + i*dy
		  line:move_to(p.x, p.y)
		  line:line_to(p.x + dx.dx, p.y)
	       end
	       ls = g.cross[segment_index(i,j,'v')]
	       if ls and ls[k] then
		  local p = left + j*dx + i*dy
		  line:move_to(p.x, p.y)
		  line:line_to(p.x, p.y + dy.dy)
	       end
	    end
	 end
	 pl:addline(line, rainbow(k+1))
      end
      pl:show()
      return pl
   end
	 
   return {cross_check  = grid_cross_check, 
	   print_cross  = debug_print_cross,
	   points       = start_point_iter,
	   cell_verify  = grid_cell_verify,
	   point_index  = grid_get_point_index,
	   remove_cross = grid_remove_cross,
	   does_close   = grid_does_close,
	   ds           = ds,
	   zstep        = zstep}
end

local function contour_step_check (s, dir, g, level)
   local p = point(s.p.x, s.p.y)
   contour_step(s, dir)

   local cell = g.point_index(p)
   cell.level = level

   if cell.dj == 0 then
      if s.p.x < p.x then cell.j = cell.j - 1 end
   end
   if cell.di == 0 then
      if s.p.y < p.y then cell.i = cell.i - 1 end
   end

   if not g.cell_verify(cell) then return 'bad' end

   if g.cross_check(cell, p, s.p) then return true end
end

local function contour_find(s, g, level)
   local ln = path()
   local p0 = s.p
 
   local closed
   for _, dir in ipairs {1, -1} do
      s.p = p0

      g.remove_cross(s.p, level)

      local a, hit
      ln:move_to(s.p.x, s.p.y)
      while true do
	 hit = contour_step_check(s, dir, g, level)

	 if hit == 'bad' then break end

	 if not hit and a then
	    closed = g.does_close(a, s.p, p0)
	    if closed then 
	       ln:close()
	       break
	    end
	 end

	 ln:line_to(s.p.x, s.p.y)

	 if hit then break end
	 a = s.p
      end

      if closed then break end
   end

   return ln
end

local function wrap_function(f)
   local pv, gv = new(2, 1), new(2, 1)
   return function(p, g)
	     pv:set(1,1, p.x)
	     pv:set(2,1, p.y)
	     if g then 
		local z = f(pv, gv)
		g.dx, g.dy = gv[1], gv[2]
		return z
	     else
		return f(pv)
	     end
	  end
end

function contour(f, av, bv, ngridx, ngridy, nlevels)
   ngridx = ngridx and ngridx or 20
   ngridy = ngridy and ngridy or 20
   nlevels = nlevels and nlevels or 12
   local a, b = point(av[1], av[2]), point(bv[1], bv[2])
   local fw = wrap_function(f)
   local g = grid_create(fw, a, b, ngridx, ngridy, nlevels)
   local pl = plot()
   for p, level in g.points() do
      local s = stepper(fw, p, g.ds, g.zstep)
      pl:addline(contour_find(s, g, level), 'gray')
   end
   pl:show()
   return pl
end
