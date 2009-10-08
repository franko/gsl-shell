-- integr module follows

local function get_spec(spec, key, mtype)
   local defaults = {eps_abs= 1e-6, eps_rel= 1e-6, limit= 512,
		     rule= 'SINGULAR'}
   local v = spec[key]
   if v and type(v) ~= mtype then 
      error(string.format('parameter %s should be of type %s', key, mtype))
   end
   return v and v or defaults[key]
end

local function type_check(v, mtype, name)
   if not v then error('parameter ' .. name .. ' required') end
   if type(v) ~= mtype then 
      error('parameter ' .. name .. ' should be of type ' .. mtype) 
   end
end

local integ_raw = integ_module_init ()

function integ(spec)
   local limit_min = 512

   local f = spec.f
   type_check (spec.f, 'function', 'f')
   if not spec.points or #spec.points < 2 then
      error('integration points not given') 
   end
   local pts = spec.points
   local inttype

   local cspec = {}
   local integ_spec_add = 
      function(key, mtype) 
	 local value = get_spec(spec, key, mtype)
	 cspec[key] = value
      end

   integ_spec_add ('eps_abs', 'number')
   integ_spec_add ('eps_rel', 'number')
   integ_spec_add ('limit',   'number')
   if cspec.limit < limit_min then cspec.limit = limit_min end

   cspec.a = pts[1]
   cspec.b = pts[#pts]

   -- infinite intervals
   if pts[1] == '-inf' or pts[#pts] == '+inf' then
      if pts[1] == '-inf' and pts[#pts] == '+inf' then
	 inttype = 'agi'
      elseif pts[1] == '-inf' then
	 inttype = 'agil'
      else
	 inttype = 'agiu'
      end
      if spec.weight then
	 local w = spec.weight
	 if w.type == 'sin' or w.type == 'cos' then
	    inttype = string.gsub(inttype, '^agi', 'awf')
	    if (inttype == 'awf') then
	       cspec.eps_abs = cspec.eps_abs / 2
	       cspec.a, cspec.b = 0, 0
	       local lint, lerr = integ_raw(f, 'awfl', cspec, w)
	       local uint, uerr = integ_raw(f, 'awfu', cspec, w)
	       return lint + uint, lerr + uerr
	    end
	    return integ_raw(f, inttype, cspec, w)
	 end
	 error('cannot calculate indefinite integral with this weight')
      end
      return integ_raw(f, inttype, cspec)
   end

   for i,v in ipairs(pts) do 
      if type(v) ~= 'number' then error('invalid points') end 
   end

   -- weighted integrals
   if spec.weight then
      local w = spec.weight
      if #pts > 2 then 
	 error('weighted integrals does not accept singular points') 
      end
      if w.type == 'sin' or w.type == 'cos' then
	 type_check (w.omega, 'number', 'omega')
	 inttype = 'awo'
      elseif w.type == 'cauchy' then
	 type_check (w.singularity, 'number', 'singularity')
	 inttype = 'awc'
      elseif w.type == 'alglog' then
	 inttype = 'aws'
      else
	 error('unknown wight type')
      end
      return integ_raw(f, inttype, cspec, w)
   end

   if not spec.adaptive then
      if #pts > 2 then 
	 error('non-adaptive integrals does not accept singular points') 
      end
      inttype = 'ng'
   end

   if #pts > 2 then
      integ_spec_add ('points', 'table')
      inttype = 'agp'
   end

   local rulename = get_spec(spec, 'rule', 'string')
   if rulename == 'SINGULAR' then
      inttype = 'ags'
   else
      cspec.rule = tonumber(string.match(rulename, 'GAUSS(%d+)'))
      if not cspec.rule then error('invalid integration rule') end
      inttype = 'ag'
   end

   return integ_raw(f, inttype, cspec)
end
