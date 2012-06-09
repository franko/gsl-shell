-- tests for complex
use'complex'
local t = {}

local a=math.pi
local b=math.sqrt(2)
local z=new(a,b)

t.new = function() return new(a, b) end
t.real = function() return real(z) end
t.imag = function() return imag(z) end
t.abs = function() return abs(z) end
t.norm2 = function() return norm2(z) end
t.rect = function() return {rect(z)} end
t.conj = function() return conj(z) end
t.sqrt = function() return sqrt(z) end
t.exp = function() return exp(z) end
t.log = function() return log(z) end
t.log10 = function() return log10(z) end
t.pow = function() return pow(z, a) end
t.sin = function() return sin(z) end
t.cos = function() return cos(z) end
t.tan = function() return tan(z) end
t.sec = function() return sec(z) end
t.csc = function() return csc(z) end
t.cot = function() return cot(z) end
t.asin = function() return asin(z) end
t.acos = function() return acos(z) end
t.atan = function() return atan(z) end
t.asec = function() return asec(z) end
t.acsc = function() return acsc(z) end
t.acot = function() return acot(z) end
t.sinh = function() return sinh(z) end
t.cosh = function() return cosh(z) end
t.tanh = function() return tanh(z) end
t.sech = function() return sech(z) end
t.csch = function() return csch(z) end
t.coth = function() return coth(z) end
t.asinh = function() return asinh(z) end
t.acosh = function() return acosh(z) end
t.atanh = function() return atanh(z) end
t.asech = function() return asech(z) end
t.acsch = function() return acsch(z) end
t.acoth = function() return acoth(z) end
return t
