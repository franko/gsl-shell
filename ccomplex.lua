
local ffi  = require 'ffi'
local cgsl = require 'cgsl'

local cmpl = ffi.typeof('complex')

local complex = {new= cmpl}

local function cartesian(x)
   if type(x) == 'number' then return x, 0 else
      return x[0], x[1]
   end
end

local function coper(op)
   return function(a, b)
	     local ar, ai = cartesian(a)
	     local br, bi = cartesian(b)
	     return op(ar,ai,br,bi)
	  end
end

local function add(ar,ai,br,bi)
   return cmpl(ar+br, ai+bi)
end

local function sub(ar,ai,br,bi)
   return cmpl(ar-br, ai-bi)
end

local function mul(ar,ai,br,bi)
   return cmpl(ar*br - ai*bi, ar*bi + ai*br)
end

local function div(ar,ai,br,bi)
   local d = br^2 + bi^2
   return cmpl((ar*br + ai*bi)/d, (-ar*bi + ai*br)/d)
end

function complex.conj(z)
   return cmpl(z[0], -z[1])
end

function complex.sqr(z)
   return z[0]^2 + z[1]^2
end

local mt = {
   __add = coper(add),
   __mul = coper(mul),
   __sub = coper(sub),
   __div = coper(div),
   __pow = function(z,n) 
	      if type(n) == 'number' then
		 return cgsl.gsl_complex_pow_real (z, n)
	      else
		 if type(z) == 'number' then z = cmpl(z,0) end
		 return cgsl.gsl_complex_pow (z, n)
	      end
	   end,
}

ffi.metatype('complex', mt)

local function cwrap(name)
   local fc = cgsl['gsl_complex_' .. name]
   local fr = math[name]
   return function(x)
	     if type(x) == 'number' then
		return fr(x)
	     else
		return fc(x)
	     end
	  end
end

function complex.real(z)
   if type(z) == 'number' then return z else return z[0] end
end

function complex.imag(z)
   if type(z) == 'number' then return 0 else return z[1] end
end

gsl_function_list = {'sqrt', 'exp', 'log', 'log10', 'sin', 'cos', 'sec', 'csc', 'tan', 'cot', 'arcsin', 'arccos', 'arcsec', 'arccsc', 'arctan', 'arccot', 'sinh', 'cosh', 'sech', 'csch', 'tanh', 'coth', 'arcsinh', 'arccosh', 'arcsech', 'arccsch', 'arctanh', 'arccoth'}

for _, name in ipairs(gsl_function_list) do
   complex[name] = cwrap(name)
end

return complex
