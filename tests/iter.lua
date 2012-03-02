-- tests for module iter
use'iter'
local tt = {}

local f=math.sin
local a=3
local b=10
local xi=8
local xs=2
local n=22

-- function tests

tt.sequence = function(test) test{ilist(sequence(f, a, b),b-a)} end
tt.sequence2 = function(test) test{ilist(sequence(f, b),b)} end
tt.sample = function(test) test{ilist(sample(f, xi, xs, n),n)} end
tt.isample = function(test) test{ilist(isample(f, a, b),b-a)} end
tt.isample2 = function(test) test{ilist(isample(f, b),b)} end
tt.ilist = function(test) test{ilist(f, a, b)} end
tt.ilist2 = function(test) test{ilist(f, b)} end
tt.isum = function(test) test{isum(f, a, b)} end
tt.isum2 = function(test) test{isum(f, b)} end

-- method tests


return tt
