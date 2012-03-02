use'matrix'
local t = {}
local d=10
local m= function() return matrix.new(d,d, |n,k| k <= n and sf.choose(n-1, k-1) or 0) end
local b= function() return vec(iter.ilist(|x| x%4+1,d)) end

t.m = function(test) test{m()} end
t.b = function(test) test{b()} end
t.inv = function(test) test{inv(m())} end
t.inv2 = function(test) test(m()*inv(m()),unit(d)) end
t.solve = function(test) test{solve(m(), b())} end
t.svd = function(test) test{svd(m())} end
return t
