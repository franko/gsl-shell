-- tests for module matrix
use'matrix'
local cexp = complex.exp
local tt = {}

local i=5
local j=6
local v=math.pi
local k0=2
local k1=3
local n0=5
local n1=4
local r=12
local c=12
local finit=|n,k| k <= n and sf.choose(n-1, k-1) or 0
local t={{1,2,3},{4,5,6},{7,8,9}}
local ve={0.1,0.01,0.001,0.0001}
local m= function() return cnew(11, 11, |j,k| cexp(2i*math.pi*(j-1)*(k-1)/11)) end
local n=15
local a= function() return new(r,c,finit) end
local b =function() return new(r,c,|n,k| math.sin(k*n*math.pi/64)) end
local f=finit

-- function tests

tt.new = function() return new(r, c, finit) end
tt.cnew = function() return cnew(r, c, finit) end
tt.def = function() return def(t) end
tt.vec = function() return vec(ve) end
tt.dim = function() return dim(m()) end
tt.copy = function() return copy(m()) end
tt.transpose = function() return transpose(m()) end
tt.hc = function() return hc(m()) end
tt.diag = function() return diag(vec(ve)) end
tt.unit = function() return unit(n) end
tt.set = function() local ax,bx=a(),b(); set(ax, bx); return ax,bx end
tt.fset = function() local mx = m(); fset(mx, f); return mx,matrix.new(11,11,f) end

-- method tests

tt.copy = function() return m():copy() end
tt.get = function() return m():get(i, j) end
tt.set_method = function() local mx=m(); mx:set(i, j, v) return mx end
tt.slice = function() return m():slice(k0, k1, n0, n1) end
tt.norm = function() return m():norm() end
tt.row = function() return m():row(i) end
tt.col = function() return m():col(j) end

return tt
