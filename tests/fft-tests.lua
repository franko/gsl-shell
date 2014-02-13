--This is a test for all the fft functions in the library

local size = 10
local size2 = 5

local r1d = matrix.new(size, 1, |i,j|i)
local c1d = matrix.cnew(size,1, |i,j|i)

local r2d = matrix.new(size,size, |i,j|i)
local c2d = matrix.cnew(size,size, |i,j|i)

local r2dasym = matrix.new(size,5, |i,j|i)
local c2dasym = matrix.new(size,5, |i,j|i)

local r3d = matrix.new(size2*size2*size2, 1, |i,j|i)
local c3d = matrix.cnew(size2*size2*size2,1, |i,j|i)

-- 1D test

print(num.fftinv(num.fft(c1d))/size)
print('---')
print(num.rfftinv(num.rfft(r1d))/size)
print('---')

-- 2D tests

print(num.fft2inv(num.fft2(c2d))/(size*size))
print('---')
print(num.rfft2inv(num.rfft2(r2d))/(size*size))
print('---')

-- 3D tests

print('---')
print(num.fftninv(num.fftn(c3d, {size2, size2, size2}), {size2, size2, size2})/(size2*size2*size2))
print('---')
print(num.rfftninv(num.rfftn(r3d, {size2, size2, size2}), {size2, size2, size2})/(size2*size2*size2))