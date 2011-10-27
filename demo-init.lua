
local demo_list = {}

local function load_demo(name)
   local record = require('demos/' .. name)
   local group, info = record[1], record[2]
   local section = demo_list[group]
   if not section then 
      section = {}
      demo_list[group] = section
   end
   local i = #section
   for k, v in ipairs(info) do 
      section[i+k] = v
   end
end

local demo_files = {
  'fft', 'bspline', 'wave-particle', 'plot', 'fractals', 'ode', 'nlinfit', 'integ', 'anim', 'linfit', 'contour'}

for i, name in ipairs(demo_files) do
   load_demo(name)
end

local function print_demos_list()
   for group, t in pairs(demo_list) do
      echo('*** ', group)
      for k, v in ipairs(t) do
	 echo(v.name .. ' - ' .. v.description)
      end
      echo ''
   end
end

local function load_demo(name)
   for group, t in pairs(demo_list) do
      for k, entry in ipairs(t) do
	 if entry.name == name then
	    return entry
	 end
      end
   end
end

return {
   list = print_demos_list,
   load = load_demo,
}
