
require 'contour/geom'

pt2  = geom.point
vec2 = geom.vector
local scalar = geom.scalarprod

frosenbrock = function(p, g)
		 local x, y = p:coords()
		 local v = 100*(y-x^2)^2 + (1-x)^2
		 if (g) then
		    g.dx = -4*100*(y-x^2)*x - 2*(1-x)
		    g.dy =  2*100*(y-x^2)
		 end
		 return v
	      end

local function quad_root_solve(f, f0, p, d)
   local pl, pr = p-d, p+d
   local fl, fc, fr = f(pl), f(p), f(pr)
   local a0, a1, a2 = (fr + 2*fc + fl)/4 - f0, (fr - fl)/2, (fr - 2*fc + fl)/4
   local q = -(a0 - a2)/a1 - 6*a2* (a0 - a2)^2 / a1^3
   return p + q * d
end

local function segment_solve(f, f0, p, d)
   local pz = quad_root_solve(f, f0, p, d)
   local g = vec2()
   local z = f(pz, g) - f0
   g = (z / g:square()) * g
   d = (g:square() / scalar(g, d)) * d
   pz = quad_root_solve(f, f0, pz - d, d)
   return pz
end

function contour_step(s, dir)
   local g, gt = vec2(), vec2()

   s.f(s.p, g)

   local gnrm = g:norm()
   local u = dir * vec2(g.dy / gnrm, - g.dx / gnrm)

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

function contour_dir(s)
   local g = vec2()
   s.f(s.p, g)
   return vec2(g.dy, -g.dx)
end

function stepper(f, p0, step, z_spacing)
   local s = {f         = f, 
	      p         = p0, 
	      z0        = f(p0), 
	      step      = step, 
	      z_spacing = z_spacing,
	      z_tol     = z_spacing * 1e-6}
   return s
end

function stepper_set(s, p)
   s.p  = p
   s.z0 = s.f(p)
end

function grid_create(f, left, right, nx, ny, nlevels)
   local g = {z= {}, cross= {}, zmin= f(left), zmax= f(right), levels= {}}
   local dx = vec2((right.x - left.x) / nx, 0)
   local dy = vec2(0, (right.y - left.y) / ny)

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
   for k=0, nlevels do g.levels[k] = g.zmin + k * zstep end

   for k, z in pairs(g.levels) do populate_grid(g, k, z) end

   local function alpha(x, a, b, s) 
      local r = (x-a)/(b-a) 
      if r > 0 and r <= 1 and (x-a)*s > 0 then return r end
   end

   local function test(a, a1, a2)
      if a then return (not a1 or a < a1) and (not a2 or a < a2) end
   end

   local function grid_remove_cross(i, j, dir, level)
      local si = segment_index(i, j, dir)
      if g.cross[si] then g.cross[si][level] = nil end
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

      if my_debug then
	 print(p1, p2, a, b)
	 print(axr); print(ayt); print(axl); print(ayb)
	 print(cell)
	 io.read('*l')
      end

      if test(axr, ayt, ayb) then
	 grid_remove_cross(cell.i, cell.j+1, 'v', cell.level)
	 cell.j = cell.j + 1
	 if (cell.j >= nx) then return cut_at_boundary(p1, p2, axr) end
	 return grid_cross_check(cell, p1, p2)
      elseif test(ayt, axl, axr) then
	 grid_remove_cross(cell.i+1, cell.j, 'h', cell.level)
	 cell.i = cell.i + 1
	 if (cell.i >= ny) then return cut_at_boundary(p1, p2, ayt) end
	 return grid_cross_check(cell, p1, p2)
      elseif test(axl, ayt, ayb) then
	 grid_remove_cross(cell.i, cell.j,   'v', cell.level)
	 cell.j = cell.j - 1
	 if (cell.j < 0) then return cut_at_boundary(p1, p2, axl) end
	 return grid_cross_check(cell, p1, p2)
      elseif test(ayb, axl, axr) then
	 grid_remove_cross(cell.i, cell.j,   'h', cell.level)
	 cell.i = cell.i - 1
	 if (cell.i < 0) then return cut_at_boundary(p1, p2, ayb) end
	 return grid_cross_check(cell, p1, p2)
      end

      return false
   end

   local function get_segment(i, j, dir, level)
      local c = {i1= i, j1= j, level= level}
      if dir == 'h' then c.i2, c.j2 = i, j+1 else c.i2, c.j2 = i+1, j end
      return c
   end

   local function grid_next_point (level)
      for i= 0, ny do
	 for j= 0, nx do
	    local si = segment_index(i, j, 'h')
	    if g.cross[si] and g.cross[si][level] then
	       grid_remove_cross(i, j, 'h', level)
	       local p = left + (j+0.5)*dx + i*dy
	       p = segment_solve(f, g.levels[level], p, 0.5 * dx)
	       return p, get_segment(i, j, 'h', level)
	    end
	    si = segment_index(i, j, 'v')
	    if g.cross[si] and g.cross[si][level] then
	       grid_remove_cross(i, j, 'v', level)
	       local p = left + j*dx + (i+0.5)*dy
	       p = segment_solve(f, g.levels[level], p, 0.5 * dy)
	       return p, get_segment(i, j, 'v', level)
	    end
	 end
      end
   end

   local function start_point_iter()
      local level = 1
      local p, sg
      return function()
	 while true do
	    p, sg = grid_next_point(level)
	    if p then break end
	    level = level + 1
	    if level >= nlevels then return end
	 end
	 return p, sg
      end
   end      

   local function grid_cell_verify(c)
      if c.j >= 0 and c.j + 1 <= nx then
	 if c.i >= 0 and c.i + 1 <= ny then
	    return true
	 end
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
	 
   return {cross_check = grid_cross_check, 
	   print_cross = debug_print_cross,
	   points      = start_point_iter,
	   cell_verify = grid_cell_verify}
end

function contour_step_check (s, dir, g, cell)
   local p = pt2(s.p.x, s.p.y)
   contour_step(s, dir)
   return g.cross_check(cell, p, s.p)
end

local function verify_if_close(a, b, c)
   if (b.x - c.x) * (a.x - c.x) <= 0 and (b.y - c.y) * (a.y - c.y) <= 0 then
      return true
   end
end

local function get_oriented_cell(sg, u)
   local c = {i= sg.i1, j= sg.j1, level= sg.level}
   if sg.i1 == sg.i2 then
      if u.dy < 0 then c.i = c.i - 1 end
   else
      if u.dx < 0 then c.j = c.j - 1 end
   end
   return c
end

function contour_find(s, g, sg)
   local ln = path()
   local u = contour_dir(s)
   local p0 = s.p
 
   local a, hit, closed
   for _, dir in ipairs {1, -1} do
      s.p = p0
      cell = get_oriented_cell(sg, u)

      if my_debug then
	 print(s.p, cell, dir, sg, u)
	 io.read('*l')
      end

      if g.cell_verify(cell) then

	 ln:move_to(s.p.x, s.p.y)

	 repeat
	    hit = contour_step_check(s, dir, g, cell)

	    if my_debug then
	       print('RESULT: ', s.p, cell, hit)
	       io.read('*l')
	    end

	    if not hit and a then
	       closed = verify_if_close(a, s.p, p0)
	       if closed then 
		  ln:close()
		  break
	       end
	    end

	    ln:line_to(s.p.x, s.p.y)
	    a = s.p
	 until hit
      end

      if closed then break end
      u, a = (-1) * u, nil
   end

   return ln
end
