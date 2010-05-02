
-- contour-impcurve.lua
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

local plcurve = require 'plcurve'

local M = {}

local insert = table.insert

local function order_add_relation(t, a, b)
   if not t[a] then t[a] = {inf= {}} end
   if not t[b] then t[b] = {inf= {}} end
   t[b].inf[a] = true
end

local function grid_create(f, left, right, nx, ny, nlevels)
   local cross, roots, lncross = {}, {}, {h= {}, v= {}}
   local g = {z= {}, zmin= f(left), zmax= f(right), levels= {}}
   local dx = vector {(right[1] - left[1]) / nx, 0}
   local dy = vector {0, (right[2] - left[2]) / ny}
   local ds = sqrt(dx[1]^2 + dy[2]^2)
   local zstep, z_eps
   local order_tree

   for i=0, ny do lncross.h[i] = {} end
   for i=0, nx do lncross.v[i] = {} end

   local DEBUG_PLOT = plot()
   local ECHO_DEBUG = false

   local function get_root(id, si)
      return roots[id][si]
   end

   local curves = {}
   local curve_id_current = 0
   local function curve_next_id() return curve_id_current + 1 end
   local function curve_register(curve)
      curve_id_current = curve_id_current + 1
      curves[curve_id_current] = curve
   end

   local function index(i,j) return j + i * (nx+1) end

   local function index_lookup(si) return divmod(si, nx+1) end

   local function segment_index_i(i1, j1, i2, j2)
      if i2 < i1 then i1, i2 = i2, i1 end
      if j2 < j1 then j1, j2 = j2, j1 end
      if i1 ~= i2 and j1 ~= j2 then error 'not a segment' end
      if i1 >= 0 and j1 >= 0 and i2 <= ny and j2 <= nx then 
	 local idx = index(i1, j1)
	 if j1 == j2 then idx = idx + (nx+1) * (ny+1) end
	 return idx
      end
   end

   local function grid_point(i, j)
      return left + j * dx + i * dy
   end

   local function grid_snap(p, epsilon)
      epsilon = epsilon and epsilon or 0
      local jr, ir = (p[1] - left[1]) / dx[1], (p[2] - left[2]) / dy[2]
      local i, j = floor(ir +  epsilon), floor(jr + epsilon)
      local di = abs(ir - i) <= epsilon and 0 or 1
      local dj = abs(jr - j) <= epsilon and 0 or 1
      return i, j, di, dj
   end

   local function segment_index_lookup_i(si)
      local t, r = divmod(si, (nx+1)*(ny+1))
      local i1, j1 = divmod(r, nx+1)
      local i2, j2 = i1, j1
      if t == 0 then j2 = j1+1 elseif t == 1 then i2 = i1+1
      else error 'invalid index' end
      return i1, j1, i2, j2
   end

   local function segment_index_lookup(si)
      local s = {}
      s.i1, s.j1, s.i2, s.j2 = segment_index_lookup_i(si)
      return s
   end

   local function bord_main_index(si)
      local i1, j1, i2, j2 = segment_index_lookup_i(si)
      local s, k
      if i1 == i2 then
	 if i1 == 0 then s, k = 0, j1 elseif i1 == ny then s, k = 2, nx-j2
	 else error 'not a boundary point' end
      else
	 if j1 == 0 then s, k = 3, ny-i2 elseif j1 == nx then s, k = 1, i1
	 else error 'not a boundary point' end
      end
      return s, k
   end

   local function bord_point_index(p)
      local epsilon = 1e-4 * ds
      local DD = {right[1] - left[1], right[2] - left[2]}
      local function islo(v, i) return v[i] < left[i]  + epsilon end
      local function ishi(v, i) return v[i] > right[i] - epsilon end
      local function dnm(v, i) return (v[i] -  left[i]) / DD[i] end
      local function inm(v, i) return (right[i] - v[i]) / DD[i] end
      if     islo(p, 2) then return dnm(p, 1)
      elseif ishi(p, 1) then return dnm(p, 2) + 1
      elseif ishi(p, 2) then return inm(p, 1) + 2
      elseif islo(p, 1) then return inm(p, 2) + 3 end
      error 'point not in boundary'
   end

   local function bord_index(i, j)
      if     i == 0  then return j
      elseif j == nx then return i + nx
      elseif i == ny then return ny + nx + (nx - j)
      elseif j == 0  then return 2*nx + ny + (ny - i)
      else error 'not a boundary point' end
   end

   local function segment_index_lookup_acw(i1, j1, i2, j2)
      if (i1 == i2 and i1 == ny) or (j1 == j2 and j1 == 0) then return i2, j2
      else return i1, j1 end
   end

   local function angle_is_between(th, th1, th2)
      if th1 <= th2 then 
	 return th > th1 and th <= th2
      else
	 return th > th1 or  th <= th2
      end
   end

   local function segment_index(s)
      return segment_index_i(s.i1, s.j1, s.i2, s.j2)
   end

   local function segment_pivot(s)
      local di, dj = s.j2 - s.j1, -(s.i2 - s.i1)
      s.i1, s.j1, s.i2, s.j2 = s.i2, s.j2, s.i2 + di, s.j2 + dj
   end

   local function segment_invert(s)
      s.i1, s.j1, s.i2, s.j2 = s.i2, s.j2, s.i1, s.j1
   end

   local function segment_copy(s)
      return {i1=  s.i1, j1= s.j1, i2= s.i2, j2= s.j2}
   end

   local function grid_intersect(si, level)
      local i1, j1, i2, j2 = segment_index_lookup_i(si)
      local z = g.levels[level]
      local p0 = grid_point((i1 + i2)/2, (j1 + j2)/2)
      local d = (j2 - j1)/2 * dx + (i2 - i1)/2 * dy
      return plcurve.segment_root(f, z, p0, d, z_eps)
   end

   local function curve_extrema(crv, dir)
      if dir > 0 then return crv.a, crv.b else return crv.b, crv.a end
   end

   local nodes, nodes_order = {}, {}
   local function add_node(p, id, level)
      local nid = #nodes+1
      nodes[nid] = {p= p, curveid= id, level= level}
      return nid
   end

   local boxx = {right[1], right[1], left[1], left[1]}
   local boxy = {left[2], right[2], right[2], left[2]}

   local function full_border_points()
      local k = 0
      return function()
		if k < 4 then
		   k = k+1
		   return boxx[k], boxy[k]
		end
	     end
   end

   local function nodes_border_points(nid1, nid2)
      local n1, n2 = nodes[nid1], nodes[nid2]

      local th1 = bord_point_index(n1.p)
      local th2 = bord_point_index(n2.p)
      if th2 < th1 then th2 = th2 + 4 end

      return function()
		local i1, i2 = floor(th1), floor(th2)
		local x, y
		if i1 < i2 then
		   x, y = boxx[(i1%4)+1], boxy[(i1%4)+1]
		   th1 = i1 + 1
		   return x, y
		elseif th1 < th2 then
		   x, y = n2.p[1], n2.p[2]
		   th1 = th2
		   return x, y
		end
	     end
   end

   local domains, domain_marks = {}, {}
   local function domain_add(dom)
      local domid = #domains + 1
      domains[domid] = dom

      local a0, a, b
      for i, join in ipairs(dom) do
	 local curve = curves[join.id]
	 local bp = b
	 a, b = curve_extrema(curve, join.direction)

	 curve.domain[join.direction]= domid

	 if i == 1 then 
	    a0 = a
	 else
	    insert(domain_marks, {domid= domid, a= bp, b= a})
	 end

	 if i == #dom then
	    insert(domain_marks, {domid= domid, a= b, b= a0})
	 end
      end
      return domid
   end

   local function bord_domain_lookup(i, j)
      local th = bord_point_index(grid_point(i, j))

      local function is_member(nid1, nid2)
	 local th1 = bord_point_index(nodes[nid1].p)
	 local th2 = bord_point_index(nodes[nid2].p)
	 return angle_is_between(th, th1, th2)
      end

      for domid, dom in ipairs(domains) do
	 local it, term = {}, nil
	 it.f, it.s, it.i = ipairs(dom)
	 it.i, term = it.f(it.s, it.i)
	 if not it.i then return domid end
	 local nid0, nid1 = curve_extrema(curves[term.id], term.direction)
	 while true do
	    it.i, term = it.f(it.s, it.i)
	    if not it.i then break end

	    local nida, nidb = curve_extrema(curves[term.id], term.direction)
	    if is_member(nid1, nida) then return domid end
	    nid1 = nidb
	 end
	 if is_member(nid1, nid0) then return domid end
      end

      error ('no domain found')
   end

   local function is_member_of_domain(domid, cid)
      for _, term in ipairs(domains[domid]) do
	 if term.id == cid then return true end
      end
   end

   local function curve_opposite_domain(id, domid)
      local curve = curves[id]
      local domn, domp = curve.domain[-1], curve.domain[1]
      if domn ~= domid and domp ~= domid then
	 error 'wrong curve/domain request' 
      end
      return (domn == domid and domp or domn)
   end

   local function node_acw_next(nid)
      local idx
      for i, k in ipairs(nodes_order) do 
	 if k == nid then
	    idx = i;
	    break
	 end
      end
      if not idx then error 'cannot find node id' end

      local nxt = idx < #nodes and idx + 1 or 1
      return nodes_order[nxt]
   end

   local function node_sort()
      for j=1,#nodes do nodes_order[j] = j end

      local function nfsort(aidx, bidx)
	 local a, b = nodes[aidx].p, nodes[bidx].p
	 return bord_point_index(a) < bord_point_index(b)
      end
      
      table.sort(nodes_order, nfsort)
   end

   local function add_cross_at_index(si, k)
      if not cross[si] then cross[si] = {} end
      cross[si][k] = 'undef'
   end

   local function assign_cross_at_index(si, k, id)
      if si and cross[si] and cross[si][k] == 'undef' then 
	 local c = curves[id]
	 cross[si][k] = id 
	 c[#c+1] = si
	 if not roots[id] then roots[id] = {} end
	 roots[id][si] = grid_intersect(si, k)
	 return true
      end
   end

   local function check_cross(idx, k, id)
      if not id then error 'curve id not specified' end
      return cross[idx] and cross[idx][k] == id 
   end

   local function add_line_cross(id, i, x, dir)
      local neps = 20
      local epsilon = (dir == 'h' and dx[1] or dy[2]) / neps
      local list = lncross[dir][i]
      for k, c in ipairs(list) do
	 if x < c[2] + epsilon then
	    if id ~= c[1] or (x < c[2] - epsilon) then
	       insert(list, k, {id, x})
	    end
	    return
	 end
      end
      insert(list, {id, x})
   end

   local function point_is_valid(i, j)
      return i >= 0 and i <= ny and j >= 0 and j <= nx
   end

   local function grid_iter_vertices()
      local i, j = 0, 0
      return function()
		if i <= ny then
		   local ia, ja = i, j
		   j = j + 1
		   if j > nx then i, j = i+1, 0 end
		   return ia, ja
		end
	     end
   end

   local function grid_iter_segments()
      local s = {}
      local cnt, n = 0, 2*nx+1
      return function()
		local q, r = divmod(cnt, n)
		local j, di, dj
		if q <= ny then
		   if r < nx then
		      j, di, dj = r, 0, 1
		   else
		      j, di, dj = r-nx, 1, 0
		   end
		   s.i1, s.j1 = q, j
		   s.i2, s.j2 = s.i1 + di, s.j1 + dj
		   cnt = cnt+1
		   if cnt <= ny * n + nx then return s end
		end
	     end
   end

   local function grid_iter_intersects(level)
      local sgm_iter = grid_iter_segments()
      return function()
		local s, si
		repeat
		   s = sgm_iter()
		   if s then si = segment_index(s) else return end
		until check_cross(si, level, 'undef')
		return s, si
	     end
   end

   for i, j in grid_iter_vertices() do
      local p = grid_point(i, j)
      local z = f(p)
      if z < g.zmin then g.zmin = z end
      if z > g.zmax then g.zmax = z end
      g.z[index(i,j)] = z
   end

   zstep = (g.zmax - g.zmin) / nlevels
   z_eps = 1e-5 * zstep
   for k=0, nlevels do g.levels[k] = g.zmin + k * zstep end

   local function grid_populate(level, z)
      for s in grid_iter_segments(level) do
	 local idx1, idx2 = index(s.i1, s.j1), index(s.i2, s.j2)
	 local z1, z2 = g.z[idx1], g.z[idx2]
	 local dz1, dz2 = z1 - z, z2 - z

	 if z1 == z then dz1 = 1 end
	 if z2 == z then dz2 = 1 end

	 if dz1 * dz2 < 0 then
	    local si = segment_index(s)
	    add_cross_at_index(si, level)
	 end
      end
   end

   for k, z in pairs(g.levels) do grid_populate(k, z) end

   local function inner_level_sign(s, level)
      local z2, zc = g.z[index(s.i2, s.j2)], g.levels[level]
      return (z2 > zc and -1 or 1)
   end

   local function find_domains()
      local mark = {}
      for id, _ in ipairs(curves) do mark[id] = 3 end

      local function curve_unmark(id, dir)
	 local m1, m2 = divmod(mark[id], 2)
	 if dir == 1 then m2 = 0 else m1 = 0 end
	 mark[id] = m1 * 2 + m2
      end

      local function domain_join(id, dir)
	 local dom, crv = {}, curves[id]
	 insert(dom, {id= id, direction= dir})
	 curve_unmark(id, dir)
	 local a, b = curve_extrema(crv, dir)

	 local i1, j1, di, dj = grid_snap(nodes[b].p)
	 local i, j = segment_index_lookup_acw(i1, j1, i1+di, j1+dj)
	 dom.level = crv.level + (dir < 0 and 0 or -1)

	 local a0 = a
	 local c = node_acw_next(b)
	 while c ~= a0 do
	    id = nodes[c].curveid
	    crv = curves[id]
	    local dir = (crv.a == c and 1 or -1)
	    insert(dom, {id= id, direction= dir})
	    curve_unmark(id, dir)
	    a, b = curve_extrema(crv, dir)
	    c = node_acw_next(b)
	 end
	 return dom
      end

      for id, crv in ipairs(curves) do
	 if not crv.closed then
	    local m1, m2 = divmod(mark[id], 2)
	    if m2 > 0 then
	       local dom = domain_join(id, 1)
	       domain_add(dom)
	    end
	    if m1 > 0 then
	       local dom = domain_join(id, -1)
	       domain_add(dom)
	    end
	 end
      end

      if #domains == 0 then 
	 local z, lev = g.z[index(0,0)], 0
	 while g.levels[lev+1] and g.levels[lev+1] < z do lev = lev+1 end
	 insert(domains, {level= lev}) 
      end
   end

   local function grid_remove_cross(p, level, id, dir)
      local i, j, di, dj = grid_snap(p, 0.1)
      local st

      if dir then
	 local k, ci
	 if dir == 'h' then k, ci = i, 1 else k, ci = j, 2 end
	 add_line_cross(id, k, p[ci], dir)
      end

      if dj == 0 then 
	 assign_cross_at_index(segment_index_i(i, j, i+1, j), level, id)
	 -- delicate affair, I'm not sure that we should keep
         -- the second condition "di == 0" 
	 if di == 0 then
	    assign_cross_at_index(segment_index_i(i-1, j, i, j), level, id)
	 end
      end
      if di == 0 then 
	 assign_cross_at_index(segment_index_i(i, j, i, j+1), level, id)
	 -- same remark as above for "dj == 0"
	 if dj == 0 then
	    assign_cross_at_index(segment_index_i(i, j-1, i, j), level, id)
	 end
      end
   end

   local function alpha(x, a, b, s) 
      local r = (x-a)/(b-a) 
      if r > 0 and r <= 1 and (x-a)*s > 0 then return r end
   end

   local function alphatest(a, a1, a2)
      if a then return (not a1 or a < a1) and (not a2 or a < a2) end
   end

   local function grid_cross_check(i, j, p1, p2, level, id)
      local a, b = grid_point(i, j), grid_point(i+1, j+1)

      local function cut_at_boundary(beta)
	 set(p2, p1 + beta * (p2 - p1))
--	 grid_remove_cross(p2, level, id)
	 return true
      end
    
      local axr = alpha(b[1], p1[1], p2[1],  1)
      local ayt = alpha(b[2], p1[2], p2[2],  1)
      local axl = alpha(a[1], p1[1], p2[1], -1)
      local ayb = alpha(a[2], p1[2], p2[2], -1)

      if alphatest(axr, ayt, ayb) then
	 grid_remove_cross(p1 + axr * (p2 - p1), level, id, 'v')
	 j = j + 1
	 if j >= nx then return cut_at_boundary(axr) end
	 return grid_cross_check(i, j, p1, p2, level, id)
      elseif alphatest(ayt, axl, axr) then
	 grid_remove_cross(p1 + ayt * (p2 - p1), level, id, 'h')
	 i = i + 1
	 if i >= ny then return cut_at_boundary(ayt) end
	 return grid_cross_check(i, j, p1, p2, level, id)
      elseif alphatest(axl, ayt, ayb) then
	 grid_remove_cross(p1 + axl * (p2 - p1), level, id, 'v')
	 j = j - 1
	 if j < 0 then return cut_at_boundary(axl) end
	 return grid_cross_check(i, j, p1, p2, level, id)
      elseif alphatest(ayb, axl, axr) then
	 grid_remove_cross(p1 + ayb * (p2 - p1), level, id, 'h')
	 i = i - 1
	 if i < 0 then return cut_at_boundary(ayb) end
	 return grid_cross_check(i, j, p1, p2, level, id)
      end

      return false
   end

   local function curve_join(s0, level, id)
      local z0 = g.levels[level]
      local si0 = segment_index(s0)

      local irt, jrt = s0.i1, s0.j1
      if s0.i1 == s0.i2 then irt = nil else irt = nil end
      local cw = 0

      local function segment_lookup(s, idx0)
	 segment_pivot(s)

	 if not point_is_valid(s.i2, s.j2) then return 'boundary' end

	 for cnt = 1,3 do
	    local si = segment_index(s)
	    if si == idx0 then return 'close' end

	    if check_cross(si, level, 'undef') then
	       assign_cross_at_index(si, level, id)
	       return 'success'
	    end

	    segment_pivot(s)
	 end

	 error 'failed to find direction'
      end

      local function run(c, s, idx0)
	 local status

	 repeat
	    status = segment_lookup(s, idx0)
	    if status == 'success' then 
	       local p
	       if     s.i1 == irt and s.i1 == s.i2 then 
		  p = (s.j1 - s0.j1) * (s.j2 - s.j1)
	       elseif s.j1 == jrt and s.j1 == s.j2 then 
		  p = (s.i1 - s0.i1) * (s.i2 - s.i1)
	       end
	       if p then cw = cw + (p < 0 and 1 or -1) end
	    end
	    segment_invert(s)
	 until status ~= 'success'

	 return status
      end

      local function curve_reverse(c)
	 local j, n = 1, #c
	 while j < n do
	    c[j], c[n] = c[n], c[j]
	    j, n = j+1, n-1
	 end
      end

      local curve = {domain= {}, level= level}
      curve_register(curve)

      assign_cross_at_index(si0, level, id)

      local s = segment_copy(s0)
      local inner = inner_level_sign(s, level)
      local status = run(curve, s, si0)
      if status == 'boundary' then
	 curve_reverse(curve)
	 s = segment_copy(s0)
	 segment_invert(s)
	 status = run(curve, s, si0)
      else
	 if cw < 0 then inner = -inner end
	 if inner < 0 then curve.level = curve.level - 1 end
	 curve.closed = cw
      end

      return curve
   end

   local function curve_join_implicit(s0, level, id)
      local z0 = g.levels[level]
      local si0 = segment_index(s0)
      local cw = 0

      local function step_check (stepper, dir)
	 local ps = stepper.point()
	 local p = ps:copy()

	 stepper.advance(dir)

	 local i, j, di, dj = grid_snap(p)

	 if dj == 0 and ps[1] < p[1] then j = j - 1 end
	 if di == 0 and ps[2] < p[2] then i = i - 1 end

	 if not (i >= 0 and i < ny and j >= 0 and j < nx) then 
	    grid_remove_cross(p, level, id)
	    stepper.set(p)
	    return true
	 end

	 if grid_cross_check(i, j, p, ps, level, id) then 
	    return true
	 end
      end

      local function run(curve, si0, dir)
	 local debug_curve = path()
	 local fake_point = path(left[1], left[2])
	 local debug_n = 0

	 local function add_point(plt, p)
	    local ln = path(p[1], p[2])
	    plt:add(ln, 'red', {{'marker', size= 5}})
	 end

	 local p0, pa = get_root(id, si0), new(2,1)
	 local stepper = plcurve.stepper(f, p0, ds, zstep)
	 local points = curve.points
	 local start = true
	 
	 local sg = segment_index_lookup(si0)
	 local oi = (sg.j1 == sg.j2 and 1 or 2)
	 local oj = (sg.j1 == sg.j2 and 2 or 1)

	 repeat
	    set(pa, stepper.point())
	    local boundary_cross = step_check (stepper, dir)
	    local pb = stepper.point()

	    if not start and plcurve.does_close(pa, pb, p0, 0.3 * ds) then
	       return true
	    end

	    insert(points, {pb[1], pb[2]})

	    if DEBUG_PLOT then
	       if start then
		  debug_curve:move_to(pa[1], pa[2])
		  debug_curve:line_to(pb[1], pb[2])
		  DEBUG_PLOT:addline(debug_curve, 'gray')
	       else
		  debug_curve:line_to(pb[1], pb[2])
		  DEBUG_PLOT:add(fake_point)
	       end

	       for k=debug_n+1, #curve do
		  local ssi = curve[k]
		  local s = segment_index_lookup(ssi)
		  local a, b = grid_point(s.i1, s.j1), grid_point(s.i2, s.j2)
		  local seg = path(a[1], a[2])
		  seg:line_to(b[1], b[2])
		  DEBUG_PLOT:addline(seg, rainbow(level))
	       end
	       debug_n = #curve
	    end

	    local dya, dyb = pa[oi] - p0[oi], pb[oi] - p0[oi]
	    if not start and (dyb * dya < 0 or dyb == 0) then
	       local sign = (oi == 1 and 1 or -1)
	       local dyi = dyb - dya
	       local a = -dya / dyi
	       local xi = pa[oj] - p0[oj] + a * (pb[oj] - pa[oj])
	       if DEBUG_PLOT then add_point(DEBUG_PLOT, pa + a * (pb - pa)) end
	       cw = cw + (sign * dyi * xi > 0 and -1 or 1)
	    end

	    start = false
	 until boundary_cross

	 return false
      end

      local function curve_reverse(c)
	 local j, n = 1, #c
	 while j < n do
	    c[j], c[n] = c[n], c[j]
	    j, n = j+1, n-1
	 end

	 local ps = c.points
	 j, n = 1, #ps
	 while j < n do
	    ps[j], ps[n] = ps[n], ps[j]
	    j, n = j+1, n-1
	 end
      end

      local curve = {domain= {}, level= level}
      curve_register(curve)

      assign_cross_at_index(si0, level, id)
      local px = get_root(id, si0)
      curve.points = { {px[1], px[2]} }

      local ss = segment_index_lookup(si0)
      grid_remove_cross(px, level, id, ss.i1 == ss.i2 and 'h' or 'v')

      if DEBUG_PLOT then
	 print('curve', id)
	 local bord = path(left[1], left[2])
	 bord:line_to(right[1], left[2])
	 bord:line_to(right[1], right[2])
	 bord:line_to(left[1],  right[2])
	 bord:close()
	 DEBUG_PLOT:addline(bord, 'blue')
	 DEBUG_PLOT:show()
      end

      local is_closed = run(curve, si0, 1)
      if not is_closed then
	 curve_reverse(curve)
	 status = run(curve, si0, -1)
      else
	 if cw < 0 then curve.level = curve.level - 1 end
	 if cw == 0 then error 'error in curve closure determination' end
	 curve.closed = cw
      end

      if DEBUG_PLOT then io.read('*l') end

      return curve
   end

   local function line_scan_points_iter(p, dir)
      local csicross = lncross[dir][p]
      local q = 1
      return function()
		if q <= #csicross then
		   local cc = csicross[q]
		   q = q+1
		   return cc[1]
		end
	     end
   end

   local function line_scan_roots_iter(si_iter, field)
      local klist, k, si
      local function self()
	 if not klist or k > #klist then
	    si = si_iter()
	    while si do
	       if cross[si] then
		  klist, k = {}, 1
		  for k, id in pairs(cross[si]) do
		     insert(klist, {level= k, id= id})
		  end
		  table.sort(klist, function(a, b)
				 local pa, pb = roots[a.id][si], roots[b.id][si]
				 return pa[field] < pb[field]
			      end)
		  return self()
	       end
	       si = si_iter()
	    end
	 else
	    local id = klist[k].id
	    k = k+1
	    return id, si
	 end
      end
      return self
   end

   local function order_curves()
      print('ORDER CURVES')

      searchlist = {}
      for id, _ in ipairs(curves) do 
	 if curves[id].closed then insert(searchlist, id) end
      end

      table.sort(searchlist, function(ia, ib) return #curves[ia] < #curves[ib] end)

      local function make_row_iter(i)
	 local j = 0
	 return function()
		   if j+1 <= nx then
		      local si = segment_index_i(i, j, i, j+1)
		      j = j+1
		      return si
		   end
		end
      end

      local function make_col_iter(j)
	 local i = 0
	 return function()
		   if i+1 <= ny then
		      local si = segment_index_i(i, j, i+1, j)
		      i = i+1
		      return si
		   end
		end
      end
      
      local order_tree = {}

      function list_val_remove(ls, id)
	 for k, cid in ipairs(ls) do
	    if cid == id then 
	       table.remove(ls, k)
	       break
	    end
	 end
      end

      while #searchlist > 0 do
	 local id = searchlist[1]
	 if not id then break end

	 local i1, j1, i2, j2 = segment_index_lookup_i(curves[id][1])

--	 local line_iter, field
	 local kf
	 if i1 == i2 then 
	    kf, j1 = line_scan_points_iter(i1, 'h'), 0
--	    line_iter, field, j1 = make_row_iter(i1), 1, 0
	    print('scan along row', i1)
	 else
	    kf, i1 = line_scan_points_iter(j1, 'v'), 0
--	    line_iter, field, i1 = make_col_iter(j1), 2, 0
	    print('scan along column', j1)
	 end

	 local st = {}
	 local domid = bord_domain_lookup(i1, j1)
	 for id, _ in kf do
	    if not curves[id].closed then
	       if not is_member_of_domain(domid, id) then
		  -- DEBUG DEBUG DEBUG
		  print('ERROR: curve', id, 'does not match domain', domid)
	       else
		  domid = curve_opposite_domain(id, domid)
		  print('> domain:', domid)
		  if #st ~= 0 then error 'wrong curve stack in curve_order' end
	       end
	    else
	       print('curve', id)
	       list_val_remove(searchlist, id)

	       if id == st[#st] then
		  table.remove(st)
		  local sup = #st > 0 and st[#st] or ('D' .. domid)
		  order_add_relation(order_tree, id, sup)
	       else
		  insert(st, id)
	       end
	    end
	 end
	 print('SCAN OK')
      end

      -- DEBUG DEBUG DEBUG
      local function set_to_string(set)
	 local tb = {}
	 for k, _ in pairs(set) do insert(tb, k) end
	 return string.format('{%s}', table.concat(tb, ','))
      end

      for id, c in pairs(order_tree) do
	 print('> curve', id, 'inf:', set_to_string(c.inf))
      end

      return order_tree
   end

   local function grid_find_curves()
      for level=1, nlevels do
	 for s, _ in grid_iter_intersects(level) do
	    local id = curve_next_id()
	    local curve = curve_join_implicit(s, level, id, true)

	    if not curve.closed then
	       local a, b = curve.points[1], curve.points[#curve.points]
	       local aid = add_node(a, id, level)
	       local bid = add_node(b, id, level)
	       curve.a, curve.b = aid, bid
	    end
	 end
      end

      node_sort()

      find_domains()

      order_tree = order_curves()
   end

   local function debug_print_cross(color_function)
      local pl = plot()
      for k = 0, nlevels do
	 local line = path()
	 local dline = path()
	 for i=0,ny do
	    for j=0,nx do
	       local ls = cross[segment_index_i(i,j,i,j+1)]
	       local p = grid_point(i, j)
	       if ls and ls[k] then
		  if ls[k] == 'undef' then
		     dline:move_to(p[1], p[2])
		     dline:line_to(p[1] + dx[1], p[2])
		  else
		     line:move_to(p[1], p[2])
		     line:line_to(p[1] + dx[1], p[2])
		  end
	       end
	       ls = cross[segment_index_i(i,j,i+1,j)]
	       if ls and ls[k] then
		  if ls[k] == 'undef' then
		     dline:move_to(p[1], p[2])
		     dline:line_to(p[1], p[2] + dy[2])
		  else
		     line:move_to(p[1], p[2])
		     line:line_to(p[1], p[2] + dy[2])
		  end
	       end
	    end
	 end
	 local col = color_function(k+1)
	 pl:addline(dline, col)
	 pl:addline(line, col, {{'dash', a=3, b=3}})
      end
      pl:show()
      return pl
   end

   local function curve_points(id, dir)
      local pts = curves[id].points
      local i, n = (dir > 0 and 1 or #pts), #pts
      return function()
		if i >= 1 and i <= n then
		   local p = pts[i]
		   i = i + dir
		   return p[1], p[2]
		end
	     end
   end

   local function line_add(ln, xyf, action)
      local x0, y0 = xyf()
      if not x0 then print 'warning: empty line'; return end
      if action ~= 'skip' then ln[action](ln, x0, y0) end
      for x, y in xyf do ln:line_to(x, y) end
   end

   local function node_xy_coord(nid)
      return nodes[nid].p[1], nodes[nid].p[2]
   end

   local function domain_draw_path(domid)
      local ln = path()
      local n0, n1
      for _, join in ipairs(domains[domid]) do
	 local na, nb = curve_extrema(curves[join.id], join.direction)
	 local action = n0 and 'skip' or 'move_to'

	 if n1 then line_add(ln, nodes_border_points(n1, na), 'line_to') end

	 line_add(ln, curve_points(join.id, join.direction), action)

	 n1 = nb
	 if not n0 then n0 = na end
      end

      if n0 then
	 line_add(ln, nodes_border_points(n1, n0), 'line_to')
      else
	 line_add(ln, full_border_points(), 'move_to')
      end

      ln:close()
      return ln
   end

   local function curve_add_path(ln, id, verse)
      local c = curves[id]
      if c.closed then
	 local vdir = (verse == 'cw' and -1 or 1)
	 local orient = vdir * c.closed
	 line_add(ln, curve_points(id, orient), 'move_to')
	 ln:close()
      else
	 line_add(ln, curve_points(id, 1), 'move_to')
      end
   end

   local function color(a)
      return rgba(0.9, 0.9 - 0.9*a, 0, 0.8)
   end

   local function curve_draw(pl, id)
      local ln = path()
      curve_add_path(ln, id, 'acw')
      local tree = order_tree[id]
      if tree then
	 for sid, _ in pairs(tree.inf) do
	    curve_add_path(ln, sid, 'cw')
	    curve_draw(pl, sid)
	 end
      end
      local a = curves[id].level/nlevels
      pl:add(ln, color(a))
   end

   local function grid_draw_regions(pl)
      for domid, dom in ipairs(domains) do
 	 local dpath = domain_draw_path(domid)
	 local tree = order_tree['D' .. domid]
	 if tree then
	    for sid, _ in pairs(tree.inf) do
	       curve_add_path(dpath, sid, 'cw')
	       curve_draw(pl, sid)
	    end
	 end
	 local a = dom.level/nlevels
	 pl:add(dpath, color(a))

	 -- DEBUG DEBUG DEBUG
	 local cls = {}
	 if tree then for sid, _ in pairs(tree.inf) do insert(cls, sid) end end
	 print('domain', domid, 'curves:', cls)
--	 io.read('*l')
      end
   end

   local function grid_draw_lines(pl, color)
      local ln = path()
      for id = 1, #curves do
	 curve_add_path(ln, id, 'cw')
      end
      pl:addline(ln, color)
   end
	 
   return {
	   print_cross    = debug_print_cross,
           find_curves    = grid_find_curves,
           draw_regions   = grid_draw_regions,
           draw_lines     = grid_draw_lines}
end

function M.plot(f, a, b, ngridx, ngridy, nlevels)
   ngridx = ngridx and ngridx or 20
   ngridy = ngridy and ngridy or 20
   nlevels = nlevels and nlevels or 12
   local g = grid_create(f, vector(a), vector(b), ngridx, ngridy, nlevels)

--   local p = g.print_cross(rainbow)
--   io.read('*l')

   g.find_curves()

   pl = plot()
   g.draw_regions(pl)
   g.draw_lines(pl, rgba(0,0,0,0.6))
   pl:show()

   return pl
end

return M
