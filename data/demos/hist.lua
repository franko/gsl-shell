
local function hist_demo()
	local r = rng.new()
	local mu, sigma = 5, 1.0
	local t = gdt.create(|| { x = rnd.gaussian(r, sigma) + mu }, 100)
	local p = gdt.hist(t, "x", {title = "Simulated gaussian mu=" .. mu .. " sigma=".. sigma, show= false})
	p:show()
end

return {'Histogram', {
  {
     name= 'hist',
     f = hist_demo,
     description = 'Histogram of simulated gaussian data',
  },
}}
