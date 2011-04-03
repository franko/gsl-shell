use 'complex'

local ffi = require 'ffi'
local cgsl = require 'cgsl'

local root = dofile('root.lua')

local rsin, rcos, rsqrt, rexp = math.sin, math.cos, math.sqrt, math.exp
local atan2, pi = math.atan2, math.pi

local function info(msg)
   io.write(msg)
   io.flush()
end

local x1 = -24
local x2 =  24

local v1 = 0
local v2 = 32

energy_limit = 80

function ks(e)
   return sqrt(2*(e-v1)), sqrt(2*(e-v2))
end

local smat = matrix.cnew(4, 4)
local bmat = matrix.vec {0,0,0,1 + 0 * complex.I}

function Asget(k1, k2, e)
   local a1, a2 = exp(I*k1*x1), exp(I*k2*x2)

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

function edet(e)
   local k1, k2 = ks(e)
   return k1 * rsin(k2*x2) * rcos(k1*x1) - k2 * rsin(k1*x1) * rcos(k2*x2)
end

function edet_sub(e)
   local k1, g2 = rsqrt(2*(e-v1)), rsqrt(2*(v2-e))
   return k1 * math.sinh(g2*x2) * rcos(k1*x1) - g2 * rsin(k1*x1) * math.cosh(g2*x2)
end

function root_grid_search(emax)
   local roots = {}
   local de = v2/4000
   local fa = edet_sub(de)
   local fb
   local ea, eb = de, de
   while eb < v2 do
      eb = eb + de
      fb = edet_sub(eb)
      if fa * fb < 0 then
	 local r = root(edet_sub, ea, eb, 1e-8, 1e-8)
--	 print('found:', r, 'between', ea, eb)
	 roots[#roots+1] = r
	 ea, fa = eb, fb
      end
   end

   ea = v2 + de
   fa = edet(ea)
   while ea < emax do
      eb = eb + de
      fb = edet(eb)
      if fa * fb < 0 then
	 roots[#roots+1] = root(edet, ea, eb, 1e-8, 1e-8)
	 ea, fa = eb, fb
      end
   end

   return matrix.vec(roots)
end

info 'Finding energy eigenvalues (roots)...'
roots = root_grid_search(energy_limit)
echo 'done'
--print(roots)

local function csqrn(z)
   return real(z) * real(z) + imag(z) * imag(z)
end

function phi_norm(e)
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

function As_mat_compute(roots)
   local n = #roots
   local m = cgsl.gsl_matrix_alloc (n, 8)
   for i=1, n do
      local e = roots[i]
      local k1, k2 = ks(e)
      local A1, A2, B1, B2 = Asget(k1, k2, e)
      local nc = rsqrt(1 / phi_norm(e))

      cgsl.gsl_matrix_set(m, i-1, 2*0    , nc * real(A1))
      cgsl.gsl_matrix_set(m, i-1, 2*0 + 1, nc * imag(A1))

      cgsl.gsl_matrix_set(m, i-1, 2*1    , nc * real(A2))
      cgsl.gsl_matrix_set(m, i-1, 2*1 + 1, nc * imag(A2))

      cgsl.gsl_matrix_set(m, i-1, 2*2    , nc * real(B1))
      cgsl.gsl_matrix_set(m, i-1, 2*2 + 1, nc * imag(B1))

      cgsl.gsl_matrix_set(m, i-1, 2*3    , nc * real(B2))
      cgsl.gsl_matrix_set(m, i-1, 2*3 + 1, nc * imag(B2))
   end

   return m
end

info 'Calculating energy eigenstates...'
local As_mat = As_mat_compute(roots)
echo 'done'

local function As_coeff(i, j)
   local ar = cgsl.gsl_matrix_get(As_mat, i, 2*j  )
   local ai = cgsl.gsl_matrix_get(As_mat, i, 2*j+1)
   return ar, ai
end

function feval(i, x)
   local e = cgsl.gsl_matrix_get (roots, i, 0)
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

function coherent_state(x0, p0, sig)
   return function(x)
	     return exp(-(x-x0)^2/(4*sig^2) + I*p0*x)
	  end
end

local initstate = {x0= -14, p0= 8, sigma= 0.8}

fcs = coherent_state(initstate.x0, initstate.p0, initstate.sigma)
pcs = graph.fxplot(|x| (csqrn(fcs(x))), x1, x2)
pcs:limits(x1, 0, x2, 1.4)
pcs.title = 'Wave function density'

function plot_roots()
   local ps = graph.fxplot(edet, v2, roots[#roots])
   ps:addline(graph.fxline(edet_sub, v1, v2), 'magenta')

   local rln = graph.path(roots[1], 0)
   for i = 2, #roots do
      rln:line_to(roots[i], 0)
   end

   ps:addline(rln, 'blue', {{'marker', size=5}})
   io.read '*l'
end

-- plot_roots()

function coeff(i)
   local p0, x0, sigma = initstate.p0, initstate.x0, initstate.sigma
   local e = roots[i]
   local k1 = rsqrt(2*(e-v1))
   local A1r, A1i = As_coeff(i-1, 0)
   local A2r, A2i = As_coeff(i-1, 1)
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

info 'Calculating initial state coefficients...'
coeffs = cgsl.gsl_vector_alloc (2 * #roots)
for i = 1, #roots do
   local cr, ci = coeff(i)
   cgsl.gsl_vector_set (coeffs, 2*(i-1)  , cr)
   cgsl.gsl_vector_set (coeffs, 2*(i-1)+1, ci)
end
echo 'done'

function plot_coeffs()
   local w = graph.window 'v..'

   local p = graph.plot()
   ln = graph.ipath(gsl.sequence(function(i) return roots[i+1], coeffs.data[2*i] end, 0, #roots-1))
   p = graph.plot()
   p:addline(ln)
   w:attach(p, 2)

   local p = graph.plot()
   ln = graph.ipath(gsl.sequence(function(i) return roots[i+1], coeffs.data[2*i+1] end, 0, #roots-1))
   p = graph.plot()
   p:addline(ln)
   w:attach(p, 1)
end

plot_coeffs()

-- state_plot()

local csexp = cgsl.gsl_vector_alloc (2 * #roots)
function coeff_inv(cs, fxv, y, t)
   local n = #roots

   for i=0, n-1 do
      local e = cgsl.gsl_matrix_get (roots, i, 0)

      local cr = cgsl.gsl_vector_get (cs, 2*i  )
      local ci = cgsl.gsl_vector_get (cs, 2*i+1)

      local exr, exi = rcos(-e*t), rsin(-e*t)

      csexp.data[2*i  ] = cr*exr - ci*exi
      csexp.data[2*i+1] = cr*exi + ci*exr
   end

   local p = y.size / 2

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

local n = #roots
local p = 512
local fxv = cgsl.gsl_vector_alloc (2 * n * p)
local y = cgsl.gsl_vector_alloc (2 * p)

local xsmp = |k| (x2-x1)*k/(p-1) + x1

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

function state_plot()
   local px = graph.plot('Eigenstates Waveforms')
   px.sync = false
   px:show()

   px:pushlayer()
   for i=0, n-1 do
      local e = roots[i+1]

      local sqr = gsl.sequence(function(k) return xsmp(k), fxv.data[2*n*k+2*i] end, 0, p-1)
      local sqi = gsl.sequence(function(k) return xsmp(k), fxv.data[2*n*k+2*i+1] end, 0, p-1)

      print('Energy:', e)

      px:clear()
      px:addline(graph.ipath(sqr), 'red')
      px:addline(graph.ipath(sqi), 'blue')
      px:flush()
      io.read '*l'
   end
end

--state_plot()

echo 'READY: press enter'
io.read '*l'

pcs.sync = false
pcs:pushlayer()

function anim()
   local col = graph.rgba(0, 0.7, 0, 0.9)
   for t= 0, 22, 0.125/8 do
      coeff_inv(coeffs, fxv, y, t)
      pcs:clear()
      local ln = graph.ipath(gsl.sequence(function(i) return xsmp(i), (y.data[2*i]^2 + y.data[2*i+1]^2) end, 0, p-1))
      pcs:add(ln, col)
      pcs:flush()
   end
end

anim()
