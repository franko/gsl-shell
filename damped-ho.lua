local ffi = require 'ffi'
local gsl = require 'gsl'
local cblas = require 'cblas'

local exp, sqrt, divmod = math.exp, math.sqrt, math.divmod

local RowMajor, Trans, NoTrans = gsl.CblasRowMajor, gsl.CblasTrans, gsl.CblasNoTrans

local N = 100
local nu = 0
local lambda, omega = 0.1, 1

local function mat_def(i, j)
   local p, r = divmod(i-1, 2)
   local q, s = divmod(j-1, 2)
   local rp, ip
   if p == q then
      rp, ip = -2*lambda*p, -omega*nu
   elseif q == p+1 then
      rp, ip = 2*lambda*sqrt((p+nu+1)*(p-nu+1)), 0
   else
      rp, ip = 0, 0
   end

   if r == s then
      return rp
   else
      return (r == 1 and ip or -ip)
   end
end

local M = matrix.new(N, N, mat_def)

local f = function(t, y, dydt)
	     cblas.cblas_dgemm(RowMajor, NoTrans, NoTrans, N, 1, N, 1.0, M.data, N, y, 1, 0.0, dydt, 1)
	  end

local alpha = 4.1 + 0.5i
local a_n = complex.norm2(alpha)

local y0 = matrix.new(N, 1, function(i)
			       local n, r = divmod(i-1, 2)
			       if r == 1 then return 0 end
			       return exp(-a_n) * a_n^n/sf.gamma(n+1)
			     end)

local pndist = graph.plot()

pndist:addline(graph.filine(|n| y0.data[2*n], 0, N/2-1))

local t0, t1, h0, tsmp = 0, 5, 1e-4, 0.004
local s = num.odevec {N= N, eps_abs= 1e-8, method='rk8pd'}
local evol = s.evolve
local ln1 = graph.path(t0, y0.data[34])
local ln2 = graph.path(t0, y0.data[40])
s:init(t0, h0, f, y0.data)
for t = tsmp, t1, tsmp do
   while s.t < t do
      evol(s, f, t)
   end
   ln1:line_to(s.t, s.y[34])
   ln2:line_to(s.t, s.y[40])
end

pndist:addline(graph.filine(|n| s.y[2*n], 0, N/2-1), 'blue')
pndist:show()

local p = graph.plot('ODE integration example')
p:addline(ln1)
p:addline(ln2, "blue")
p:show()
