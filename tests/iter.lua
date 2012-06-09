-- tests for module iter
local ilist,sequence,sample,isample,isum =
  iter.ilist,iter.sequence,iter.sample,iter.isample,iter.isum
local tt = {}

local f=math.sin
local a=3
local b=10
local xi=8
local xs=2
local n=22

-- function tests

tt.sequence = function() return ilist(sequence(f, a, b),b-a) end
tt.sequence2 = function() return ilist(sequence(f, b),b) end
tt.sample = function() return ilist(sample(f, xi, xs, n),n) end
tt.isample = function() return ilist(isample(f, a, b),b-a) end
tt.isample2 = function() return ilist(isample(f, b),b) end
tt.ilist = function() return ilist(f, a, b) end
tt.ilist2 = function() return ilist(f, b) end
tt.isum = function() return isum(f, a, b) end
tt.isum2 = function() return isum(f, b) end


return tt
