require('strict').enable()

local gsl_roots = require 'roots'

local rsin, rcos, rsqrt, rexp = math.sin, math.cos, math.sqrt, math.exp
local atan2, pi = math.atan2, math.pi
local cexp, csqrt = complex.exp, complex.sqrt
local real, imag = complex.real, complex.imag
local sequence = iter.sequence

local I = 1i

local function info(msg)
   io.write(msg)
   io.flush()
end

local x1 = -24
local x2 =  24

local v1 = 0
local v2 = 32

local energy_limit = 80

local function ks(e)
   return csqrt(2*(e-v1)), csqrt(2*(e-v2))
end

local smat = matrix.cnew(4, 4)
local bmat = matrix.cnew(4, 1, |i| i == 4 and 1 or 0)

local function Asget(k1, k2, e)
   local a1, a2 = cexp(I*k1*x1), cexp(I*k2*x2)

   smat:set(1,1, 1/a1)
   smat:set(1,2, a1)
   smat:set(2,3, 1/a2)
   smat:set(2,4, a2)
   smat:set(3,1,  1)
   smat:set(3,2,  1)
   smat:set(3,3, -1)
   smat:set(3,4, -1)
   smat:set(4,2, 1)

   local x = matrix.solve(smat, bmat)
   return x[1], x[2], x[3], x[4]
end

local function edet(e)
   local k1, k2 = rsqrt(2*(e-v1)), rsqrt(2*(e-v2))
   return k1 * rsin(k2*x2) * rcos(k1*x1) - k2 * rsin(k1*x1) * rcos(k2*x2)
end

local function edet_scale(e)
   local k1 = rsqrt(2*(e-v1))
   return k1
end

local function edet_sub(e)
   local k1, g2 = rsqrt(2*(e-v1)), rsqrt(2*(v2-e))
   return k1 * math.sinh(g2*x2) * rcos(k1*x1) - g2 * rsin(k1*x1) * math.cosh(g2*x2)
end

local function edet_sub_scale(e)
   local k1, g2 = rsqrt(2*(e-v1)), rsqrt(2*(v2-e))
   return math.max(k1, g2) * rexp(g2*x2)
end

local function root_grid_search(emax)
   local emin = (pi/x1)^2/2

   local s1 = gsl_roots.solver(edet_sub, 1e-8, v2 * 1e-10, edet_sub_scale)
   local roots = s1:solve(v1 + emin/10, v2 - emin/10)

   local s2 = gsl_roots.solver(edet, 1e-8, v2 * 1e-10, edet_scale)
   s2:solve(v2 + emin/10, emax, roots)

   return matrix.vec(roots)
end

local roots

local function get_root(i)
   return roots.data[i-1]
end

local function csqrn(z)
   return real(z) * real(z) + imag(z) * imag(z)
end

local function phi_norm(e)
   local k1, k2 = ks(e)
   local A1, A2, B1, B2 = Asget(k1, k2, e)
   local phi1, phi2, dphi

   local A1s, A2s = csqrn(A1), csqrn(A2)
   phi1, phi2 = atan2(imag(A1), real(A1)), atan2(imag(A2), real(A2))
   dphi = phi2 - phi1
   local n1 = (-x1) * (A1s + A2s) + rsqrt(A1s * A2s)/k1 * (rsin(dphi) - rsin(2*k1*x1 + dphi))

   local B1s, B2s = csqrn(B1), csqrn(B2)
   phi1, phi2 = atan2(imag(B1), real(B1)), atan2(imag(B2), real(B2))
   dphi = phi2 - phi1

   local n2
   if imag(k2) == 0 then
      n2 = x2 * (B1s + B2s) + rsqrt(B1s * B2s)/k2 * (rsin(2*k2*x2 + dphi) - rsin(dphi))
   else
      local g2 = imag(k2)
      n2 = B1s * (rexp(2*g2*x2) - 1)/(2*g2) + B2s * (1-rexp(-2*g2*x2))/(2*g2) + 2 * rsqrt(B1s * B2s) * rcos(dphi) * x2
   end
   
   return n1 + n2
end

local function As_mat_compute(roots)
   local n = #roots
   local m = matrix.alloc (n, 8)
   local set = m.set
   for i=1, n do
      local e = get_root(i)
      local k1, k2 = ks(e)
      local A1, A2, B1, B2 = Asget(k1, k2, e)
      local nc = rsqrt(1 / phi_norm(e))

      local is = (i-1)*8
      m.data[is + 0] = nc * real(A1)
      m.data[is + 1] = nc * imag(A1)
      m.data[is + 2] = nc * real(A2)
      m.data[is + 3] = nc * imag(A2)
      m.data[is + 4] = nc * real(B1)
      m.data[is + 5] = nc * imag(B1)
      m.data[is + 6] = nc * real(B2)
      m.data[is + 7] = nc * imag(B2)
   end

   return m
end

local As_mat

local function As_coeff(i, j)
   local ar = As_mat.data[i*8+2*j  ]
   local ai = As_mat.data[i*8+2*j+1]
   return ar, ai
end

local function feval(i, x)
   local e = get_root(i+1)
   if e > v2 then
      if x < 0 then
	 local k1 = rsqrt(2*(e-v1))
	 local A1r, A1i = As_coeff(i, 0)
	 local A2r, A2i = As_coeff(i, 1)
	 local c, s = rcos(k1*x), rsin(k1*x)
	 return A1r*c + A1i*s + A2r*c - A2i*s, -A1r*s + A1i*c + A2r*s + A2i*c
      else
	 local k2 = rsqrt(2*(e-v2))
	 local B1r, B1i = As_coeff(i, 2)
	 local B2r, B2i = As_coeff(i, 3)
	 local c, s = rcos(k2*x), rsin(k2*x)
	 return B1r*c + B1i*s + B2r*c - B2i*s, -B1r*s + B1i*c + B2r*s + B2i*c
      end
   else
      if x < 0 then
	 local k1 = rsqrt(2*(e-v1))
	 local A1r, A1i = As_coeff(i, 0)
	 local A2r, A2i = As_coeff(i, 1)
	 local c, s = rcos(k1*x), rsin(k1*x)
	 return A1r*c + A1i*s + A2r*c - A2i*s, -A1r*s + A1i*c + A2r*s + A2i*c
      else
	 local g2 = rsqrt(2*(v2-e))
	 local B1r, B1i = As_coeff(i, 2)
	 local B2r, B2i = As_coeff(i, 3)
	 local c = rexp(g2*x)
	 return B1r*c + B2r/c, B1i*c + B2i/c
      end
   end
end

local function coherent_state(x0, p0, sig)
   return function(x)
	     return cexp(-(x-x0)^2/(4*sig^2) + I*p0*x)
	  end
end

local initstate
local fcs

local function plot_roots()
--   local ell, erl = v1, roots[n-1]
   local ell, erl = 25, 35
   local n = #roots
   local lno = graph.fxline(|x| edet(x)/edet_scale(x), v2, erl)
   local lnd = graph.fxline(|x| edet_sub(x)/edet_sub_scale(x), ell, v2)

   local ps = graph.plot()
   ps:addline(lno, 'red')
   ps:addline(lnd, 'magenta')

   local rln = graph.path(get_root(1), 0)
   for i = 2, n do
      rln:line_to(get_root(i), 0)
   end

   ps:addline(rln, 'blue', {{'marker', size=5}})
   ps:show()
   ps:limits(ell, -1, erl, 1)
end

local function coeff(i)
   local p0, x0, sigma = initstate.p0, initstate.x0, initstate.sigma
   local e = get_root(i+1)
   local k1 = rsqrt(2*(e-v1))
   local A1r, A1i = As_coeff(i, 0)
   local A2r, A2i = As_coeff(i, 1)
   local pp, pm = p0 + k1, p0 - k1
   local egp = rexp(- pp^2 * sigma^2)
   local egm = rexp(- pm^2 * sigma^2)
   local epr, epi = rcos(pp*x0), rsin(pp*x0)
   local emr, emi = rcos(pm*x0), rsin(pm*x0)
   local zr = egp * (A1r*epr + A1i*epi) + egm * (A2r*emr + A2i*emi)
   local zi = egp * (A1r*epi - A1i*epr) + egm * (A2r*emi - A2i*emr)
   local s = rsqrt(4*pi) * sigma
   return s * zr, s * zi
end

local coeffs

local function plot_coeffs()
   local w = graph.window 'v..'

   local ln = graph.ipath(sequence(function(i) return get_root(i+1), coeffs.data[2*i] end, 0, #roots-1))
   local p = graph.plot()
   p:addline(ln)
   w:attach(p, 2)

   ln = graph.ipath(sequence(function(i) return get_root(i+1), coeffs.data[2*i+1] end, 0, #roots-1))
   local p = graph.plot()
   p:addline(ln)
   w:attach(p, 1)
end

-- state_plot()

local n, p

local csexp
local function coeff_inv(cs, fxv, y, t)
   for i=0, n-1 do
      local e = get_root(i+1)
      local cr, ci = cs.data[2*i], cs.data[2*i+1]
      local exr, exi = rcos(-e*t), rsin(-e*t)

      csexp.data[2*i  ] = cr*exr - ci*exi
      csexp.data[2*i+1] = cr*exi + ci*exr
   end

   for k=0, p-1 do
      local sr, si = 0, 0
      for i=0, n-1 do
	 local cr, ci = csexp.data[2*i], csexp.data[2*i+1]
	 local fr, fi = fxv.data[2*n*k + 2*i], fxv.data[2*n*k + 2*i + 1]

	 sr = sr + (cr*fr - ci*fi)
	 si = si + (cr*fi + ci*fr)
      end

      y.data[2*k  ] = sr
      y.data[2*k+1] = si
   end
end

local fxv, y

local function xsmp(k)
   return (x2-x1)*k/(p-1) + x1
end

local function state_plot()
   local px = graph.plot('Eigenstates Waveforms')
   px.sync = false
   px:show()

   px:pushlayer()
   for i=0, n-1 do
      local e = get_root(i+1)

      local sqr = sequence(function(k) return xsmp(k), fxv.data[2*n*k+2*i] end, 0, p-1)
      local sqi = sequence(function(k) return xsmp(k), fxv.data[2*n*k+2*i+1] end, 0, p-1)

      print('Energy:', e)

      px:clear()
      px:addline(graph.ipath(sqr), 'red')
      px:addline(graph.ipath(sqi), 'blue')
      px:flush()
      io.read '*l'
   end
end

--state_plot()

local function anim(pcs)
   local col = graph.rgba(0, 0.7, 0, 0.9)
   for t= 0, 22, 0.125/8 do
      coeff_inv(coeffs, fxv, y, t)
      pcs:clear()
      local ln = graph.ipath(sequence(function(i) return xsmp(i), (y.data[2*i]^2 + y.data[2*i+1]^2) end, 0, p-1))
      pcs:add(ln, col)
      pcs:flush()
   end
end

local function wave_demo()
   info 'Finding energy eigenvalues (roots)...'
   roots = root_grid_search(energy_limit)
   echo 'done'

   info 'Calculating energy eigenstates...'
   As_mat = As_mat_compute(roots)
   echo 'done'

   initstate = {x0= -14, p0= 8, sigma= 1.5}

   fcs = coherent_state(initstate.x0, initstate.p0, initstate.sigma)

   plot_roots()

   info 'Calculating initial state coefficients...'
   coeffs = matrix.alloc(2 * #roots, 1)
   for i = 0, #roots - 1 do
      local cr, ci = coeff(i)
      coeffs.data[2*i  ] = cr
      coeffs.data[2*i+1] = ci
   end
   echo 'done'

   plot_coeffs()

   n = #roots
   p = 512

   csexp = matrix.alloc(2 * #roots, 1)

   fxv = matrix.alloc(2 * n * p, 1)
   y = matrix.alloc(2 * p, 1)

   info 'Computing eigenstates x representation...'
   for k= 0, p-1 do
      local x = xsmp(k)
      for i= 0, n-1 do
	 local fr, fi = feval(i, x)
	 fxv.data[2*n*k + 2*i    ] = fr
	 fxv.data[2*n*k + 2*i + 1] = fi
      end
   end
   echo 'done'

   local pcs = graph.canvas()
   pcs:limits(x1, 0, x2, 1.4)
   pcs:addline(graph.fxline(|x| (csqrn(fcs(x))), x1, x2))
   pcs.title = 'Wave function density'
   pcs:show()

   pcs:pushlayer()

   echo 'READY: press enter'
   io.read '*l'

   anim(pcs)
end

return {'Wave Packet', {
  {
     name = 'wave',
     f = wave_demo, 
     description = 'Quantum Wave function for a particle in a step potential'
  },
}}
