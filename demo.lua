
local demo_list = {}

local function load_demo(name)
   local group, info = dofile('demos/' .. name .. '.lua')
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

local demo_files = {'fft', 'bspline', 'wave-particle', 'plot'}

for i, name in ipairs(demo_files) do
   load_demo(name)
end

local function print_demos()
   for group, t in pairs(demo_list) do
      echo('*** ', group)
      for k, v in ipairs(t) do
	 echo(v.name .. ' - ' .. v.description)
      end
      echo ''
   end
end

function demo(name)
   if name == 'list' then
      print_demos()
   else
      for group, t in pairs(demo_list) do
	 for k, entry in ipairs(t) do
	    if entry.name == name then
	       echo(entry.description)	    
	       return entry.f()
	    end
	 end
      end
      error [[
  No such demo.
  Please type demo('list') for a list of all available demos
            ]]
   end
end

echo 'Available demos:'
echo ''
print_demos()

echo [[
  Write :

    > demo('name')

  to execute any of them.
]]
