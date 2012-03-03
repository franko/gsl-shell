-- tests for module rng
use'rng'
local tt = {}

local n=1
local seed=1
local instance = function(name) return new(name) end -- initialize instance!

-- function tests

tt.list = function(test) test{list()} end


-- method tests
for i,name in ipairs(list()) do
  local r = instance(name)
  tt[name] = function(test)
    r:set(1234567890)
    test{r:get(),r:getint(2^32-1),r:getint(2^24-1),r:getint(2^16-1),r:getint(1024),r:get()}
  end
end

return tt
