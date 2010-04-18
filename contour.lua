
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

local plcurve = require "plcurve"

local M = {}

local insert = table.insert

local function order_add_relation(t, a, b)
   if not t[a] then t[a] = {value= a, inf= {}} end
   if not t[b] then t[b] = {value= b, inf= {}} end
   t[b].inf[a] = true
end

local function grid_create(f, left, right, nx, ny, nlevels)
   local cross, roots = {}, {}
   local g = {z= {}, zmin= f(left), zmax= f(right), levels= {}}
   local dx = vector {(right[1] - left[1]) / nx, 0}
   local dy = vector {0, (right[2] - left[2]) / ny}
   local ds = sqrt(dx[1]^2 + dy[2]^2)
   local zstep, z_eps
   local order_tree

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
      if i1 ~= i2 and j1 ~= j2 then error 'not a segment' end
      if i2 < i1 then i1, i2 = i2, i1 end
      if j2 < j1 then j1, j2 = j2, j1 end
      local idx = index(i1, j1)
      if j1 == j2 then idx = idx + (nx+1) * (ny+1) end
      return idx
   end

   local function grid_point(i, j)
      return left + j * dx + i * dy
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

   local function bord_index(i, j)
      if     i == 0  then return j
      elseif j == nx then return i + nx
      elseif i == ny then return ny + nx + (nx - j)
      elseif j == 0  then return 2*nx + ny + (ny - i)
      else error 'not a boundary point' end
   end

   local function segment_index_lookup_acw(si)
      local i1, j1, i2, j2 = segment_index_lookup_i(si)
      if (i1 == i2 and i1 == ny) or (j1 == j2 and j1 == 0) then return i2, j2
      else return i1, j1 end
   end

   local function bord_index_acw(si)
      return bord_index(segment_index_lookup_acw(si))
   end

   local function bord_index_is_between(bi, bi1, bi2)
      if bi1 <= bi2 then 
	 return bi > bi1 and bi <= bi2
      else
	 return bi > bi1 or  bi <= bi2
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
   local function add_node(si, id, level)
      local p = get_root(id, si)
      local nid = #nodes+1
      nodes[nid] = {si= si, curveid= id, level= level, x= p[1], y= p[2]}
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

      local s1, k1 = bord_main_index(n1.si)
      local s2, k2 = bord_main_index(n2.si)
      if s2 < s1 or (s1 == s2 and k2 < k1) then s2 = s2 + 4 end

      return function()
		local x, y
		if s1 < s2 then
		   x, y = boxx[(s1%4)+1], boxy[(s1%4)+1]
		   s1 = s1 + 1
		   return x, y
		elseif n2 then
		   x, y = n2.x, n2.y
		   n2 = nil
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
      local bi = bord_index(i, j)

      local function is_member(nid1, nid2)
	 local si1, si2 = nodes[nid1].si, nodes[nid2].si
	 local bi1, bi2

	 bi1 = bord_index_acw(si1)
	 bi2 = bord_index_acw(si2)

	 return bord_index_is_between(bi, bi1, bi2)
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
	    if is_member(nid1, nida) then
	       return domid
	    end
	    nid1 = nidb
	 end
	 if is_member(nid1, nid0) then
	    return domid
	 end
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
	 local a, b = nodes[aidx], nodes[bidx]
	 local bia = bord_index_acw(a.si)
	 local bib = bord_index_acw(b.si)

	 if bia - bib ~= 0 then 
	    return bia < bib
	 else
	    local s = bord_main_index(a.si)
	    if     s == 0 then return a.x < b.x
	    elseif s == 1 then return a.y < b.y
	    elseif s == 2 then return a.x > b.x
	    elseif s == 3 then return a.y > b.y
	    else error 'invalid segment index in node_sort' end
	 end
      end
      
      table.sort(nodes_order, nfsort)
   end

   local function add_cross_at_index(si, k)
      if not cross[si] then cross[si] = {} end
      cross[si][k] = 'undef'
   end

   local function assign_cross_at_index(idx, k, id)
      if cross[idx] then 
	 if cross[idx][k] ~= 'undef' then error 'invalid intersection' end
	 cross[idx][k] = id 
      end
   end

   local function check_cross(idx, k, id)
      if not id then error 'curve id not specified' end
      return cross[idx] and cross[idx][k] == id 
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

   local function find_inner_level(s, level)
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

	 local i, j = segment_index_lookup_acw(nodes[b].si)
	 local zb, zc = g.z[index(i, j)], g.levels[crv.level]
	 dom.level = crv.level + (zb < zc and 0 or -1)

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

   local function curve_join(s0, level, id)
      local z0 = g.levels[level]
      local si0 = segment_index(s0)

      local irt, jrt = s0.i1, s0.j1
      if s0.i1 == s0.i2 then irt = nil else irt = nil end
      local cw = 0

      local function curve_add_point(c, si)
	 c[#c+1] = si
	 if not roots[id] then roots[id] = {} end
	 roots[id][si] = grid_intersect(si, level)
      end

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
	       curve_add_point(c, segment_index(s))

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

      assign_cross_at_index(si0, level, id)
      curve_add_point(curve, si0)

      local s = segment_copy(s0)
      local inner = find_inner_level(s, level)
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

	 local line_iter, field
	 if i1 == i2 then 
	    line_iter, field, j1 = make_row_iter(i1), 1, 0
	 else
	    line_iter, field, i1 = make_col_iter(j1), 2, 0
	 end

	 local st = {}
	 local domid = bord_domain_lookup(i1, j1)
	 for id, si in line_scan_roots_iter(line_iter, field) do
	    if not curves[id].closed and is_member_of_domain(domid, id) then
	       domid = curve_opposite_domain(id, domid)
	       if #st ~= 0 then error 'wrong curve stack in curve_order' end
	    else
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
      end
      return order_tree
   end

   local function grid_find_curves()
      for level=1, nlevels do
	 for s, _ in grid_iter_intersects(level) do
	    local id = curve_next_id()
	    local curve = curve_join(s, level, id)

	    if not curve.closed then
	       if #curve < 2 then 
		  error 'curve with points < 2 in grid_find_curves'
	       end
	       local a, b = curve[1], curve[#curve]
	       local aid = add_node(a, id, level)
	       local bid = add_node(b, id, level)
	       curve.a, curve.b = aid, bid
	    end

	    curve_register(curve)
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
      local curve = curves[id]
      local i, n = (dir > 0 and 1 or #curve), #curve
      return function()
		if i >= 1 and i <= n then
		   local p = get_root(id, curve[i])
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
      return nodes[nid].x, nodes[nid].y
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
