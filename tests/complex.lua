use'complex'
local t = {}

local a=math.pi
local b=math.sqrt(2)
local z=new(a,b)

t.new = function(test) test{new(a, b)} end
t.real = function(test) test{real(z)} end
t.imag = function(test) test{imag(z)} end
t.abs = function(test) test{abs(z)} end
t.norm2 = function(test) test{norm2(z)} end
t.rect = function(test) test{rect(z)} end
t.conj = function(test) test{conj(z)} end
t.sqrt = function(test) test{sqrt(z)} end
t.exp = function(test) test{exp(z)} end
t.log = function(test) test{log(z)} end
t.log10 = function(test) test{log10(z)} end
t.pow = function(test) test{pow(z, a)} end
t.sin = function(test) test{sin(z)} end
t.cos = function(test) test{cos(z)} end
t.tan = function(test) test{tan(z)} end
t.sec = function(test) test{sec(z)} end
t.csc = function(test) test{csc(z)} end
t.cot = function(test) test{cot(z)} end
t.asin = function(test) test{asin(z)} end
t.acos = function(test) test{acos(z)} end
t.atan = function(test) test{atan(z)} end
t.asec = function(test) test{asec(z)} end
t.acsc = function(test) test{acsc(z)} end
t.acot = function(test) test{acot(z)} end
t.sinh = function(test) test{sinh(z)} end
t.cosh = function(test) test{cosh(z)} end
t.tanh = function(test) test{tanh(z)} end
t.sech = function(test) test{sech(z)} end
t.csch = function(test) test{csch(z)} end
t.coth = function(test) test{coth(z)} end
t.asinh = function(test) test{asinh(z)} end
t.acosh = function(test) test{acosh(z)} end
t.atanh = function(test) test{atanh(z)} end
t.asech = function(test) test{asech(z)} end
t.acsch = function(test) test{acsch(z)} end
t.acoth = function(test) test{acoth(z)} end
return t
