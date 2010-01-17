
frosenbrock = function(p, g)
		 local x, y = p:coords()
		 local v = 100*(y-x^2)^2 + (1-x)^2
		 if (g) then
		    g.dx = -4*100*(y-x^2)*x - 2*(1-x)
		    g.dy =  2*100*(y-x^2)
		 end
		 return v
	      end

function module2d()
   local point_mt = {}
   point_mt.__index = point_mt

   local vector_mt = {}
   vector_mt.__index = vector_mt

   function point(x, y)
      local p = {x= x, y= y}
      setmetatable(p, point_mt)
      return p
   end

   function vector(dx, dy)
      local v = {dx= dx, dy= dy}
      setmetatable(v, vector_mt)
      return v
   end

   function scalarprod(u, v) return u.dx*v.dx + u.dy*v.dy end

   point_mt.coords = function(p)
			return p.x, p.y
		     end

   vector_mt.__mul = function(a, b)
			return vector(a * b.dx, a * b.dy)
		     end

   point_mt.__add = function(p, v)
		       return point(p.x + v.dx, p.y + v.dy)
		    end

   point_mt.__sub = function(p, v)
		       return point(p.x - v.dx, p.y - v.dy)
		    end

   vector_mt.norm   = function(v) return sqrt(v.dx^2 + v.dy^2) end
   vector_mt.square = function(v) return v.dx^2 + v.dy^2 end
end

module2d()

local function quad_root_solve(f, f0, p, d)
   local pl, pr = p-d, p+d
   local fl, fc, fr = f(pl), f(p), f(pr)
   local a0, a1, a2 = (fr + 2*fc + fl)/4 - f0, (fr - fl)/2, (fr - 2*fc + fl)/4
   local q = -(a0 - a2)/a1 - 6*a2* (a0 - a2)^2 / a1^3
   return p + q * d
end

local function segment_solve(f, f0, p, d)
   local pz = quad_root_solve(f, f0, p, d)
   local g = vector()
   local z = f(pz, g) - f0
   print('looking between:',p-d,p+d)
--   print('first appr:', f(pz), 'target', f0)
--   print('gradient:', g)
   g = (z / g:square()) * g
--   print('step:', g)
   d = (g:square() / scalarprod(g, d)) * d
--   print('gradient projection:', d)
   pz = quad_root_solve(f, f0, pz - d, d)
--   print('second appr:', f(pz), 'target', f0)
   print('found:',pz)
   return pz
end

function contour_step(s, dir)
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

function contour_dir(s)
   local g = vector()
   s.f(s.p, g)
   return vector(g.dy, -g.dx)
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

debug_cross = path()

function grid_create(f, left, right, nx, ny, nlevels)
   local g = {z= {}, cross= {}, zmin= f(left), zmax= f(right), levels= {}}
   local dx = vector((right.x - left.x) / nx, 0)
   local dy = vector(0, (right.y - left.y) / ny)

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
	       print(string.format('H: %f < %f < %f', a, z, b), i, j)
	       add_cross(g, i, j, 'h', k)
	    end
	    local a, b = edge_values(i, j, 'v')
	    if a and z >= a and z < b then
	       print(string.format('V: %f < %f < %f', a, z, b), i, j)
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
      if r > 0 and r < 1 and (x-a)*s > 0 then return r end
   end

   local function test(a, a1, a2)
      if a then return (not a1 or a < a1) and (not a2 or a < a2) end
   end

   local function grid_remove_cross(i, j, dir, level)
      local si = segment_index(i, j, dir)
      local p = left + j*dx + i*dy
      debug_cross:move_to(p.x, p.y)
      p = p + (dir == 'h' and dx or dy)
      debug_cross:line_to(p.x, p.y)
      if g.cross[si] then g.cross[si][level] = nil end
   end

   local function grid_cross_check(cell, p1, p2)
      if cell.i > ny or cell.i < 0 or cell.j > nx or cell.i < 0 then
	 return true -- true= it is outside of the boundaries
      end

      local a = left + cell.j * dx + cell.i * dy
      local b = a + dx + dy

      local axr = alpha(b.x, p1.x, p2.x,  1)
      local ayt = alpha(b.y, p1.y, p2.y,  1)
      local axl = alpha(a.x, p1.x, p2.x, -1)
      local ayb = alpha(a.y, p1.y, p2.y, -1)

      -- print(p1, p2, a, b)
      -- print(axr); print(ayt); print(axl); print(ayb)
      -- print(cell)
      -- io.read('*l')

      if test(axr, ayt, ayb) then
	 grid_remove_cross(cell.i, cell.j+1, 'v', cell.level)
	 cell.j = cell.j + 1
	 return grid_cross_check(cell, p1, p2)
      elseif test(ayt, axl, axr) then
	 grid_remove_cross(cell.i+1, cell.j, 'h', cell.level)
	 cell.i = cell.i + 1
	 return grid_cross_check(cell, p1, p2)
      elseif test(axl, ayt, ayb) then
	 grid_remove_cross(cell.i, cell.j,   'v', cell.level)
	 cell.j = cell.j - 1
	 return grid_cross_check(cell, p1, p2)
      elseif test(ayb, axl, axr) then
	 grid_remove_cross(cell.i, cell.j,   'h', cell.level)
	 cell.i = cell.i - 1
	 return grid_cross_check(cell, p1, p2)
      end

      return false
   end

   local function inner_cell(i, j, level)
      if j+1 > nx then j = j-1 end
      if i+1 > ny then i = i-1 end
      return {i= i, j= j, level= level}
   end

   local function grid_next_point (level)
      for i= 0, ny do
	 for j= 0, nx do
	    local si = segment_index(i, j, 'h')
	    if g.cross[si] and g.cross[si][level] then
	       grid_remove_cross(i, j, 'h', level)
	       local p = left + (j+0.5)*dx + i*dy
	       --		  print(p, f(p - 0.5*dx), f(p + 0.5*dx))
	       p = segment_solve(f, g.levels[level], p, 0.5 * dx)
	       return p, inner_cell(i, j, level)
	    end
	    si = segment_index(i, j, 'v')
	    if g.cross[si] and g.cross[si][level] then
	       grid_remove_cross(i, j, 'v', level)
	       local p = left + j*dx + (i+0.5)*dy
--		  print(p, f(p - 0.5*dy), f(p + 0.5*dy))
	       p = segment_solve(f, g.levels[level], p, 0.5 * dy)
	       return p, inner_cell(i, j, level)
	    end
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
	       if ls and ls[k] then print('cross level:',k,i,j,'h',ls[k]) end
	       if ls and ls[k] then
		  local p = left + j*dx + i*dy
		  line:move_to(p.x, p.y)
		  line:line_to(p.x + dx.dx, p.y)
	       end
	       ls = g.cross[segment_index(i,j,'v')]
	       if ls and ls[k] then print('cross level:',k,i,j,'v',ls[k]) end
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
	   next        = grid_next_point}
end

function contour_step_check (s, dir, g, cell)
   local p = point(s.p.x, s.p.y)
   contour_step(s, dir)
   return g.cross_check(cell, p, s.p)
end

function contour_find(s, ucap, g, cell)
   local ln = path(s.p.x, s.p.y)
   local p0 = s.p

   local u = contour_dir(s)
   local dir = scalarprod(u, ucap) > 0 and 1 or -1

   local hit = contour_step_check(s, dir, g, cell)
   if hit then ln:line_to(s.p.x, s.p.y); return p end
 
   local first = true
   while true do
      a = s.p
      hit = contour_step_check(s, dir, g, cell)
      if hit then ln:line_to(s.p.x, s.p.y); return ln end
      if (s.p.x - p0.x) * (a.x - p0.x) <= 0 and
         (s.p.y - p0.y) * (a.y - p0.y) <= 0 then
	 ln:close()
	 break
      end
      ln:line_to(s.p.x, s.p.y)
   end

   return ln
end
