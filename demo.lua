
local demo_list = {}

local function load_demo(name)
   local t = dofile('demos/' .. name .. '.lua')
   for k, v in pairs(t) do demo_list[k] = v end
end

local demo_files = {'fft', 'bspline', 'wave-particle'}

for i, name in ipairs(demo_files) do
   load_demo(name)
end

local function print_demos()
   for k, v in pairs(demo_list) do
      echo(k .. ' - ' .. v.description)
   end
end

function demo(name)
   if name == 'list' then
      print_demos()
   else
      local entry = demo_list[name]
      if not entry then
	 error [[No such demo.
	         Please type demo('list') for a list of all available demos
	       ]]
      end
      echo(entry.description)	    
      return entry.f()
   end
end

echo 'Available demos:'
echo ''
print_demos()
echo ''
echo 'Write demo \'name\' to execute any of them.'
