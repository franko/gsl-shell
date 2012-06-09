local rng=require"rng"
-- tests for module rng
local tt = {}
local instance = rng.new
local seed = 1234567890

-- function tests
tt.list = function() return rng.list() end

-- method tests
for i,name in ipairs(rng.list()) do
  local r = instance(name)
  tt[name] = function()
    r:set(seed)
    return {r:get(),r:getint(2^32-1),r:getint(2^24-1),r:getint(2^16-1),r:getint(1024),r:get()}
  end
end

return tt
