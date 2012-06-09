use'matrix'
local t = {}
local d=10
local m= function() return new(d,d, |n,k| k <= n and sf.choose(n-1, k-1) or 0) end
local b= function() return vec(iter.ilist(|x| x%4+1,d)) end

t.m = function() return m() end
t.b = function() return b() end
t.inv = function() return inv(m()) end
t.inv2 = function() return m()*inv(m()), unit(d) end
t.solve = function() return solve(m(), b()) end
t.svd = function() return {svd(m())} end
return t
