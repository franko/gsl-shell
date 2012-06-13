
local fails = {}

local test_nb = select('#', ...)
for k = 1, test_nb do
   local module_name = select(k, ...)
   local tests = assert(dofile('tests/' .. module_name .. '.lua'))
   echo('------ ' .. module_name .. ' ------')
   for name, test in pairs(tests) do
      local errmsg = test()
      if errmsg then
	 echo('FAIL', name, errmsg)
	 fails[#fails+1] = module_name .. "." .. name
      else
	 echo('PASS', name)
      end
   end
end

echo()
echo("--------- SUMMARY ---------")

if #fails > 0 then
   echo("Some tests failed: [" .. table.concat(fails, ", ") .. "].")
else
   echo("All tests suceeded.")
end
