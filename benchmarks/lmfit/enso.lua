
local x0 = matrix.vec { 10.0, 3.0, 0.5, 44.0, -1.5, 0.5, 26.0, 0.1, 1.5 }

local x = matrix.vec {
  1.0510749193E+01, 
  3.0762128085E+00,
  5.3280138227E-01,
  4.4311088700E+01,
 -1.6231428586E+00,
  5.2554493756E-01,
  2.6887614440E+01,
  2.1232288488E-01,
  1.4966870418E+00 }

local sumsq = 7.8853978668E+02

local sigma = matrix.vec {
 1.7488832467E-01,
 2.4310052139E-01,
 2.4354686618E-01,
 9.4408025976E-01,
 2.8078369611E-01,
 4.8073701119E-01,
 4.1612939130E-01,
 5.1460022911E-01,
 2.5434468893E-01 }

local F = matrix.vec {
    12.90000, 
    11.30000, 
    10.60000, 
    11.20000, 
    10.90000, 
    7.500000, 
    7.700000, 
    11.70000, 
    12.90000, 
    14.30000, 
    10.90000, 
    13.70000, 
    17.10000, 
    14.00000, 
    15.30000, 
    8.500000, 
    5.700000, 
    5.500000, 
    7.600000, 
    8.600000, 
    7.300000, 
    7.600000, 
    12.70000, 
    11.00000, 
    12.70000, 
    12.90000, 
    13.00000, 
    10.90000, 
   10.400000, 
   10.200000, 
    8.000000, 
    10.90000, 
    13.60000, 
   10.500000, 
    9.200000, 
    12.40000, 
    12.70000, 
    13.30000, 
   10.100000, 
    7.800000, 
    4.800000, 
    3.000000, 
    2.500000, 
    6.300000, 
    9.700000, 
    11.60000, 
    8.600000, 
    12.40000, 
   10.500000, 
    13.30000, 
   10.400000, 
    8.100000, 
    3.700000, 
    10.70000, 
    5.100000, 
   10.400000, 
    10.90000, 
    11.70000, 
    11.40000, 
    13.70000, 
    14.10000, 
    14.00000, 
    12.50000, 
    6.300000, 
    9.600000, 
    11.70000, 
    5.000000, 
    10.80000, 
    12.70000, 
    10.80000, 
    11.80000, 
    12.60000, 
    15.70000, 
    12.60000, 
    14.80000, 
    7.800000, 
    7.100000, 
    11.20000, 
    8.100000, 
    6.400000, 
    5.200000, 
    12.00000, 
   10.200000, 
    12.70000, 
   10.200000, 
    14.70000, 
    12.20000, 
    7.100000, 
    5.700000, 
    6.700000, 
    3.900000, 
    8.500000, 
    8.300000, 
    10.80000, 
    16.70000, 
    12.60000, 
    12.50000, 
    12.50000, 
    9.800000, 
    7.200000, 
    4.100000, 
    10.60000, 
   10.100000, 
   10.100000, 
    11.90000, 
    13.60000, 
    16.30000, 
    17.60000, 
    15.50000, 
    16.00000, 
    15.20000, 
    11.20000, 
    14.30000, 
    14.50000, 
    8.500000, 
    12.00000, 
    12.70000, 
    11.30000, 
    14.50000, 
    15.10000, 
   10.400000, 
    11.50000, 
    13.40000, 
    7.500000, 
   0.6000000, 
   0.3000000, 
    5.500000, 
    5.000000, 
    4.600000, 
    8.200000, 
    9.900000, 
    9.200000, 
    12.50000, 
    10.90000, 
    9.900000, 
    8.900000, 
    7.600000, 
    9.500000, 
    8.400000, 
    10.70000, 
    13.60000, 
    13.70000, 
    13.70000, 
    16.50000, 
    16.80000, 
    17.10000, 
    15.40000, 
    9.500000, 
    6.100000, 
   10.100000, 
    9.300000, 
    5.300000, 
    11.20000, 
    16.60000, 
    15.60000, 
    12.00000, 
    11.50000, 
    8.600000, 
    13.80000, 
    8.700000, 
    8.600000, 
    8.600000, 
    8.700000, 
    12.80000, 
    13.20000, 
    14.00000, 
    13.40000, 
    14.80000 }

local function iter()
   local i, n = 0, #F
   return function()
	     i = i + 1
	     if i <= n then
		return i, F[i]
	     end
	  end
end

return {title='ENSO dataset from NIST',
	iter= iter,
	N=168, P= 9,
	F= F,
	xref= x, x0= x0,
	t0= 1, t1= 168}