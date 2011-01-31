-- test complex library

------------------------------------------------------------------------------
require"complex"

print(complex.version)

z=complex.new(3,4)
print(z,"ABS",z:abs())
print(z,"CONJ",z:conj())
print(z,"CABS",z:conj():abs())
print(z,"NEG",-z)

z=complex.new(0,math.pi)
print(z,"EXP",z:exp())

z=complex.new(0,2*math.pi/3)
print(z,"EXP",z:exp(),(2*z:exp():imag())^2)

--I=complex.new(0,1)
I=complex.I

z=1
for n=0,7 do
	print(n,"MUL",z,"POW",I^n)
	z=z*I
end

z=3+4*I
print(z,"ABS",z:abs())
print(z,"ARG",z:arg())
print(z,"SQRT",z:sqrt())
z=2+I
print(z,"SQR",z^2)
print(z,"CUB",z^3)

z=2+11*I
print(z,"CBRT",z^(1/3))

print(complex.version)
------------------------------------------------------------------------------
