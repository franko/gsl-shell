-- tests for module matrix
use'matrix'
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
local m= function() return matrix.cnew(11, 11, |j,k| complex.exp(2i*math.pi*(j-1)*(k-1)/11)) end
local n=15
local a= function() return matrix.new(r,c,finit) end
local b =function() return matrix.new(r,c,|n,k| math.sin(k*n*math.pi/64)) end
local f=finit

-- function tests

tt.new = function(test) test{new(r, c, finit)} end
tt.cnew = function(test) test{cnew(r, c, finit)} end
tt.def = function(test) test{def(t)} end
tt.vec = function(test) test{vec(ve)} end
tt.dim = function(test) test{dim(m())} end
tt.copy = function(test) test{copy(m())} end
tt.transpose = function(test) test{transpose(m())} end
tt.hc = function(test) test{hc(m())} end
tt.diag = function(test) test{diag(vec(ve))} end
tt.unit = function(test) test{unit(n)} end
tt.set = function(test) local ax,bx=a(),b(); set(ax, bx); test(ax,bx) end
tt.fset = function(test) local mx = m(); fset(mx, f); test(mx,matrix.new(11,11,f)) end

-- method tests

tt.copy = function(test) test{m():copy()} end
tt.get = function(test) test{m():get(i, j)} end
tt.set_method = function(test) local mx=m(); mx:set(i, j, v) ; test(mx) end
tt.slice = function(test) test{m():slice(k0, k1, n0, n1)} end
tt.norm = function(test) test{m():norm()} end
tt.row = function(test) test{m():row(i)} end
tt.col = function(test) test{m():col(j)} end

return tt
